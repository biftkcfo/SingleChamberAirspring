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

    // --- 1. 仿真设置 (台架测试) ---
    double dt = 0.001;          // 采样时间 1ms
    double total_time = 5.0;    // 测试时长 5秒 (足够跑5个完整周期)

    // --- 2. 台架激励参数 (Bench Excitation) ---
    // 模拟正弦波作动测试：振幅 30mm, 频率 1.0Hz
    double bench_amp = 0.03;    // 30mm
    double bench_freq = 1.0;    // 1.0Hz (标准低频特性测试)

    // --- 3. 控制指令 (全部归零，保持被动状态) ---
    std::array<double, 4> flow_rate_cmd = {0.0, 0.0, 0.0, 0.0};   // 气阀关闭
    std::array<double, 4> cdc_current_cmd = {0.0, 0.0, 0.0, 0.0}; // 电流为0 (或设为0.4测试基础阻尼)

    // --- 4. 数据记录初始化 ---
    std::ofstream outFile("active_chassis_data.csv");
    // 保持与 Python 脚本兼容的表头
    outFile << "Time,Speed_kmh,TargetHeight,ActualHeight_FL,AirForce_FL,CDCForce_FL,Current_FL,BodyAcc,BodyVel,"
            << "FlowRate_FL,Stiffness_FL,AirDamping_FL" << std::endl;

    std::cout << "Starting Air Spring Bench Test Simulation..." << std::endl;
    std::cout << "Excitation: Sine Wave (+/- " << bench_amp * 1000 << "mm, " << bench_freq << "Hz)" << std::endl;

    for (double t = 0; t <= total_time; t += dt) {

        // ============================================================
        // 1. 生成台架激励 (直接控制行程 x_s)
        // ============================================================

        // 位移输入: x = A * sin(2*pi*f*t)
        // 注意：在这里，我们定义"压缩"为正还是"拉伸"为正取决于您的坐标系
        // 通常：z_road向上为正。
        // 这里直接模拟悬架压缩量 (suspension_deflection)
        // 设：正值 = 悬架被压缩 (Bump)，负值 = 悬架被拉伸 (Rebound)
        double deflection = bench_amp * std::sin(2 * M_PI * bench_freq * t);

        // 速度输入: v = A * w * cos(w*t)
        double velocity = bench_amp * (2 * M_PI * bench_freq) * std::cos(2 * M_PI * bench_freq * t);

        // 填入四轮数组 (虽然我们只看左前轮 FL)
        std::array<double, 4> x_s = {deflection, deflection, deflection, deflection};
        std::array<double, 4> dx_s = {velocity, velocity, velocity, velocity};

        // ============================================================
        // 2. 物理模型解算 (Physics Solver)
        // ============================================================

        // A. 计算空气弹簧力 (核心目标)
        // 输入流率为0 -> 模拟封闭气室特性
        std::array<double, 4> F_air = mySuspension.F_active_air_spring(flow_rate_cmd, x_s, dx_s, dt);

        // B. 计算 CDC 阻尼力 (可选，仅作参考)
        std::array<double, 4> F_cdc = mySuspension.F_cdc_damper(cdc_current_cmd, dx_s);

        // ============================================================
        // 3. 数据记录 (适配 Python 脚本)
        // ============================================================

        // 为了适配之前的 plot 代码，我们将一些不相关的整车变量填为 0
        double dummy_speed = 0.0;
        double dummy_acc = 0.0;

        outFile << t << ","
                       << dummy_speed << ","          // Speed_kmh (无用)
                       << 0.0 << ","                  // TargetHeight (无用)
                       << x_s[0] << ","               // ActualHeight_FL (关键：台架位移)
                       << F_air[0] << ","             // AirForce_FL (关键：弹簧力)
                       << F_cdc[0] << ","             // CDCForce_FL
                       << cdc_current_cmd[0] << ","   // Current_FL
                       << dummy_acc << ","            // BodyAcc
                       << dx_s[0] << ","              // BodyVel (这里存悬架速度)
                       << flow_rate_cmd[0] << ","     // FlowRate
                       << mySuspension.debug_k1_FL << "," // 刚度
                       << mySuspension.debug_c1_FL        // 阻尼
                       << std::endl;
    }

    std::cout << "Bench Test Finished. Data saved to active_chassis_data.csv" << std::endl;
    return 0;
}