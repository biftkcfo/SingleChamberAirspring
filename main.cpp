#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <array>
#include <filesystem>
#include "AirSpringThermodynamic.h"

// 简单的限幅函数
double clip(double n, double lower, double upper) {
    return std::max(lower, std::min(n, upper));
}

int main() {
    Suspension mySuspension;

    // --- 仿真设置 ---
    double dt = 0.001;          // 仿真步长 1ms (快速环周期)
    double total_time = 5.0;    // 模拟 4秒
    double t_slow_loop = 0.0;   // 慢速环计时器
    double slow_loop_period = 0.2; // 慢速环周期 200ms (5Hz)

    // --- 车辆状态变量 ---
    double vehicle_speed_kmh = 0.0;   // 车速
    double target_height_m = 0.0;     // 目标相对高度 (0为初始位置)

    // --- 控制器输出变量 ---
    std::array<double, 4> flow_rate_cmd = {0.0, 0.0, 0.0, 0.0}; // 空气弹簧充放气指令
    std::array<double, 4> cdc_current_cmd = {0.0, 0.0, 0.0, 0.0}; // CDC 电流指令

    // 数据记录
    std::ofstream outFile("active_chassis_data.csv");
    outFile << "Time,Speed_kmh,TargetHeight,ActualHeight_FL,AirForce_FL,CDCForce_FL,Current_FL,BodyAcc,BodyVel,"
            << "FlowRate_FL,Stiffness_FL,AirDamping_FL" << std::endl;
    // 模拟车身垂直运动 (简化为单自由度用于测试算法)
    // 真实情况这里应该是 7自由度整车模型积分
    double z_body = 0.0;
    double vz_body = 0.0;
    double az_body = 0.0;
    double mass_quarter = 450.0; // 1/4车重 kg
    double filtered_height = 0.0; // 存储滤波后的平滑高度
    double filter_alpha = 0.0001;  // 滤波系数 (越小越平滑)

    std::cout << "Starting Active Chassis Simulation..." << std::endl;

    for (double t = 0; t <= total_time; t += dt) {
        // --- --- --- --- --- --- --- --- --- --- --- --- --- ---
        // --- 1. 环境输入模拟 ---
        // --- --- --- --- --- --- --- --- --- --- --- --- --- ---

        // 正弦波路面输入
        double z_road = 0.02 * std::sin(2 * M_PI * 1.5 * t);
       double vz_road = 0.02 * 2 * M_PI * 1.5 * std::cos(2 * M_PI * 1.5 * t);

/*
        //平路
       double z_road = 0.0;
        double vz_road = 0.0;
*/
/*
        // 模拟车速变化：前2秒低速30km/h，后2秒急加速，线性加速到130km/h
        if (t < 2.0) vehicle_speed_kmh = 30.0;
        else vehicle_speed_kmh = 30.0 + (t - 2.0) * 50.0;
*/

        //模拟车速变化：匀速60km/h
        vehicle_speed_kmh = 60.0;
        // 计算相对运动 (悬架输入)
        double suspension_deflection = z_road - z_body; // 压缩为正
        // 假设四轮一致
        std::array<double, 4> x_s = {suspension_deflection, suspension_deflection, suspension_deflection, suspension_deflection};
        std::array<double, 4> dx_s = {vz_road - vz_body, vz_road - vz_body, vz_road - vz_body, vz_road - vz_body};

        // 慢速回路 (Slow Loop) - 空气弹簧高度控制 (5Hz)
        t_slow_loop += dt;
        filtered_height = filtered_height * (1.0 - filter_alpha) + x_s[0] * filter_alpha;
        if (t_slow_loop >= slow_loop_period) {
            t_slow_loop = 0.0; // 重置计时器

            // 1. 【策略层】：根据车速决定“我想去哪儿” (Target)
            if (vehicle_speed_kmh > 60.0) {
                target_height_m = 0.02; // 高速：目标是降低20mm (x_s = 0.02)
            } else {
                target_height_m = 0.0;  // 低速：目标是原高度 (x_s = 0.0)
            }

            // 2. 【执行层】：根据偏差决定“该干什么” (Action)
            // error > 0 意味着 x_s > target (车太矮了/被压太多) -> 需要充气
            // error < 0 意味着 x_s < target (车太高了) -> 需要放气

            double current_deflection = filtered_height;
            double error = current_deflection - target_height_m;
            double deadband = 0.01; // 10mm 死区 (防止频繁抖动)
            double flow_val = 0.0;

            if (error > deadband) {
                // 车身太低 -> 充气 (Inhale)
                flow_val = 0.002;
            }
            else if (error < -deadband) {
                // 车身太高 -> 放气 (Exhale)
                flow_val = -0.002;
            }
            else {
                // 高度合适 -> 保持 (Hold)
                flow_val = 0.0;
            }
                flow_rate_cmd = {flow_val, flow_val, flow_val, flow_val};  // 将计算出的 flow_val 应用到所有轮子
        }

        // 快速回路 (Fast Loop) - CDC 天棚阻尼控制 (1000Hz)
        // 逻辑：读取加速度 -> 积分得速度 -> 计算 Skyhook 阻尼 -> 输出电流

        // A. 状态估计 (State Estimation)
        // 实际中通过加速度积分，这里直接用仿真变量 vz_body
        double v_body = vz_body;
        double v_rel = dx_s[0];  // 相对速度

        // B. 天棚控制算法 (Skyhook Logic)
        // F_sky = C_sky * v_body
        double C_sky = 3000.0; // 天棚阻尼系数
        double F_target = C_sky * v_body; // 理想阻尼力

        // C. 半主动逻辑判断 (Semi-Active Constraint)
        // 如果 F_target 与 v_rel 同向 (做功为负，耗能)，则减震器可以执行
        // 否则输出最小阻尼
        double current_out = 0.0;

        // 判据：(理想力 * 相对速度 > 0) 意味着理想力方向与相对运动方向相同(阻碍运动)
        // CDC 只能产生阻碍运动的力
        // 注意方向定义：F_cdc 通常定义为阻碍压缩/拉伸

        // 简化版 Skyhook On-Off 控制：
        // 如果 车身向上运动 (v_body > 0) 且 悬架在拉伸 (v_rel < 0) -> 需要拉住车身 -> 高阻尼
        // 如果 车身向下运动 (v_body < 0) 且 悬架在压缩 (v_rel > 0) -> 需要撑住车身 -> 高阻尼
        // 综合：v_body * v_rel < 0 (基于标准坐标系)
        // 或者是 v_body * (v_body - v_wheel) > 0 ...

        if (v_body * v_rel > 0) { // 同号
            // 需要大阻尼 (High State)
            // 将理想力映射为电流 (简单比例映射)
            // 假设 1.6A 对应最大阻尼
            double gain = 0.01;
            current_out = std::abs(F_target) * gain;
        } else {
            // 需要小阻尼 (Low State)
            current_out = 0.0; // 最小电流
        }

        // 限幅 (CDC 电流范围通常 0 - 1.6A)
        current_out = clip(current_out, 0.0, 1.6);

        cdc_current_cmd = {current_out, current_out, current_out, current_out};
 /*
        // ============================================================
        // [RL 接口] 强化学习动作 -> 物理电流映射
        // ============================================================

        // 1. 获取 RL 的动作 (假设动作空间是 -1.0 到 1.0 的标量)
        // 在 main.cpp 里测试时，我们可以手动给一个值，或者写一个随机数模拟 RL
        // 在您的 RL 环境里，这里应该是： double rl_action = action[0];
        double rl_action = 0.0; // <--- 这里填入 RL 算法输出的动作值

        // 2. 将无量纲动作映射到真实电流 (例如 0A ~ 1.8A)
        // 映射公式：Action(-1) -> 0A, Action(1) -> 1.8A
        double max_current = 1.8;
        double target_current = (rl_action + 1.0) / 2.0 * max_current;

        // 3. 安全限幅 (防止 RL 输出非法值炸掉硬件)
        if (target_current < 0.0) target_current = 0.0;
        if (target_current > 1.8) target_current = 1.8;

        // 4. 下发指令
        cdc_current_cmd = {target_current, target_current, target_current, target_current};
*/
        // ============================================================
        // 物理模型解算 (Physics Solver)
        // ============================================================

        // 1. 计算空气弹簧力 (输入：充放气指令)
        std::array<double, 4> F_air = mySuspension.F_active_air_spring(flow_rate_cmd, x_s, dx_s, dt);

        // 2. 计算 CDC 阻尼力 (输入：电流指令)
        std::array<double, 4> F_cdc = mySuspension.F_cdc_damper(cdc_current_cmd, dx_s);

        // 3. 计算合力与车身动力学 (简单的 1/4 车模型用于闭环演示)
        // F_total = F_air + F_cdc
        double F_total_FL = F_air[0] + F_cdc[0];

        // 牛顿第二定律: m * a = F_total - m * g
        // 注意符号：假设向上为正。
        // F_air 是支撑力(向上为正)，F_cdc 是阻尼力
        // 重力向下
        double F_gravity = mass_quarter * 9.81;
        // 这里的 F_air 包含了静平衡力，所以 F_total - F_gravity 就是合外力
        double net_force = F_total_FL - F_gravity;

        az_body = net_force / mass_quarter;
        vz_body += az_body * dt;
        z_body += vz_body * dt;

        // 数据记录
        outFile << t << ","
                       << vehicle_speed_kmh << ","
                       << (vehicle_speed_kmh > 60 ? -0.015 : 0.0) << ","
                       << z_body << ","
                       << F_air[0] << ","
                       << F_cdc[0] << ","
                       << cdc_current_cmd[0] << ","
                       << az_body << ","
                       << vz_body << ","
                       << flow_rate_cmd[0] << ","
                       << Suspension::AirSpringParams::debug_k1_FL << ","
                       << Suspension::AirSpringParams::debug_c1_FL
                       << std::endl;
    }

    std::cout << "Simulation Finished. Data saved." << std::endl;
    std::cout << "文件保存绝对路径: "
              << std::filesystem::absolute("active_chassis_data.csv")
              << std::endl;
    return 0;
}