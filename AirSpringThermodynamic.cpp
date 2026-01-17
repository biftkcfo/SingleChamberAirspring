#include "AirSpringThermodynamic.h"
#include <iostream>
#include <cmath>
// --- 参数初始化 (空气弹簧) ---
double Suspension::AirSpringParams::P_b0_f = 5.1e5;
double Suspension::AirSpringParams::P_b0_r = 5.1e5;
double Suspension::AirSpringParams::V_b0_f = 2.45e-3;
double Suspension::AirSpringParams::V_b0_r = 2.45e-3;
double Suspension::AirSpringParams::T_atm = 298.0;
double Suspension::AirSpringParams::R_gas = 287.0;
double Suspension::AirSpringParams::Gamma = 1.4;
double Suspension::AirSpringParams::Cv = 717.5;
double Suspension::AirSpringParams::Kb_f = 135.0;
double Suspension::AirSpringParams::Kb_r = 135.0;
double Suspension::AirSpringParams::A_eff_slope_f = -0.009545;
double Suspension::AirSpringParams::A_eff_slope_r = -0.009545;
double Suspension::AirSpringParams::A_eff_int_f = 0.013;
double Suspension::AirSpringParams::A_eff_int_r = 0.013;
double Suspension::AirSpringParams::H0_f = 0.23;
double Suspension::AirSpringParams::H0_r = 0.23;
// --- 参数初始化 (CDC) ---
double Suspension::CDCSuspensionParams::Eta_MR_f = 0.0197;
double Suspension::CDCSuspensionParams::Eta_MR_r = 0.0197;
double Suspension::CDCSuspensionParams::L_MR_f = 0.4;
double Suspension::CDCSuspensionParams::L_MR_r = 0.4;
double Suspension::CDCSuspensionParams::Rd_MR_f = 0.076;
double Suspension::CDCSuspensionParams::Rd_MR_r = 0.076;
double Suspension::CDCSuspensionParams::Ap_MR_f = 0.00226708;
double Suspension::CDCSuspensionParams::Ap_MR_r = 0.00226708;
double Suspension::CDCSuspensionParams::Td_MR_f = 0.001;
double Suspension::CDCSuspensionParams::Td_MR_r = 0.001;
double Suspension::CDCSuspensionParams::C0_f = 0.1;
double Suspension::CDCSuspensionParams::C0_r = 0.1;
double Suspension::CDCSuspensionParams::C1_f = 0.15;
double Suspension::CDCSuspensionParams::C1_r = 0.15;
double Suspension::CDCSuspensionParams::C2_f = 0.000116;
double Suspension::CDCSuspensionParams::C2_r = 0.000116;
double Suspension::CDCSuspensionParams::C3_f = 0.000000105;
double Suspension::CDCSuspensionParams::C3_r = 0.000000105;

// 1. 主动空气弹簧逻辑 (支持充放气)
/**
 * @brief 计算主动空气弹簧力
 * @param control_flow_rate 充放气质量流率 (kg/s) [慢速环输入]
 * @param suspension_x_s 悬架动行程 （m)
 * @param suspension_dx_s 悬架动速度 （m/s）
 * @param dt 时间步长 (s)
 */
std::array<double, 4> Suspension::F_active_air_spring(
        const std::array<double, 4> &control_flow_rate,
        const std::array<double, 4> &suspension_x_s,
        const std::array<double, 4> &suspension_dx_s,
        const double &dt) {

    std::array<double, 4> F_total{};
        constexpr double P_atm = 101325.0;

    // 初始化质量
    if (!AirSpringParams::is_initialized) {
        double m0_f = (AirSpringParams::P_b0_f * AirSpringParams::V_b0_f) / (AirSpringParams::R_gas * AirSpringParams::T_atm);
        double m0_r = (AirSpringParams::P_b0_r * AirSpringParams::V_b0_r) / (AirSpringParams::R_gas * AirSpringParams::T_atm);
        AirSpringParams::current_gas_mass = {m0_f, m0_f, m0_r, m0_r};
        AirSpringParams::is_initialized = true;
    }

    for (size_t i = 0; i < 4; ++i) {
        const bool front_rear = (i < 2);
        // 参数选择
        const double V_b0_geom = front_rear ? AirSpringParams::V_b0_f : AirSpringParams::V_b0_r;
        const double Kb = front_rear ? AirSpringParams::Kb_f : AirSpringParams::Kb_r;
        const double Slope_A = front_rear ? AirSpringParams::A_eff_slope_f : AirSpringParams::A_eff_slope_r;
        const double Int_A = front_rear ? AirSpringParams::A_eff_int_f : AirSpringParams::A_eff_int_r;
        const double H0 = front_rear ? AirSpringParams::H0_f : AirSpringParams::H0_r;
        AirSpringParams::current_gas_mass[i] += control_flow_rate[i] * dt;// [慢速环核心] 更新气体质量
        if (AirSpringParams::current_gas_mass[i] < 1e-6) AirSpringParams::current_gas_mass[i] = 1e-6;
        const double P_active = (AirSpringParams::current_gas_mass[i] * AirSpringParams::R_gas * AirSpringParams::T_atm) / V_b0_geom; // 计算当前基准压力 (Active Pressure)
        const double Current_H = H0 - suspension_x_s[i];
        double A_eff = Slope_A * Current_H + Int_A;
        double Current_V_b = V_b0_geom - A_eff * suspension_x_s[i];
        if (Current_V_b < 1e-6) Current_V_b = 1e-6;
        const double Current_P_b = P_active * std::pow(V_b0_geom / Current_V_b, AirSpringParams::Gamma);
        const double K1 = (AirSpringParams::Gamma * Current_P_b * std::pow(A_eff, 2)) / Current_V_b;
        double C1 = 0.0;
        if (Kb > 1e-9) C1 =((AirSpringParams::Cv * AirSpringParams::current_gas_mass[i]) / Kb) * K1;
        if (i == 0) {
            AirSpringParams::debug_k1_FL = K1;
            AirSpringParams::debug_c1_FL = C1;
        }
        // 1. 面积限幅保护
        if (A_eff < 1e-5) A_eff = 1e-5;
        // 2. 体积限幅保护
        if (Current_V_b < 1e-6) Current_V_b = 1e-6;
        // 3. 重新计算受限后的绝对压力 (基于绝热过程 PV^gamma = C)
        //    Safe_P_b 是当前时刻气囊内的真实绝对压力
        double Safe_P_b = P_active * std::pow(V_b0_geom / Current_V_b, AirSpringParams::Gamma);
        // 4. 计算总支撑力 = 弹性力 + 热力学阻尼力
        //    F = (P - P_atm) * A + c1 * v
        double F_elastic = (Safe_P_b - P_atm) * A_eff;
        double F_damping = C1 * suspension_dx_s[i];
        F_total[i] = F_elastic + F_damping;
        // 5. 更新调试变量 (供 Python 画图用)
        if (i == 0) {
            // 计算等效刚度仅供观察 (不参与物理结算，所以即使是负的也不会导致仿真炸)
            // 刚度 = 压力刚度项 + 面积刚度项
            double K_pressure = (AirSpringParams::Gamma * Safe_P_b * A_eff * A_eff) / Current_V_b;
            double K_area = (Safe_P_b - P_atm) * (-Slope_A);
            AirSpringParams::debug_k1_FL = K_pressure + K_area;
            AirSpringParams::debug_c1_FL = C1;
        }
    }
    return F_total;
}
// 2. CDC 减震器逻辑
/**
 * @brief
 * @param suspension_current | 悬架电流          | A  |
 * @param suspension_dx_s    | 悬架相对速度       | m/s  |
 * @param vehicle_x_s        | 未使用            |   |
 * @param vehicle_body_vx    | 车辆纵向速度       | m/s  |
*/
std::array<double, 4> Suspension::F_cdc_damper(
        const std::array<double, 4> &suspension_current,
        const std::array<double, 4> &suspension_dx_s) {

    std::array<double, 4> F_c{}, F_MR{}, F_total{};

    for (size_t i = 0; i < 4; ++i) {
        const bool front_rear = (i < 2);

        // 参数选择
        const double Eta_MR = front_rear ? CDCSuspensionParams::Eta_MR_f : CDCSuspensionParams::Eta_MR_r;
        const double L_MR = front_rear ? CDCSuspensionParams::L_MR_f : CDCSuspensionParams::L_MR_r;
        const double Rd_MR = front_rear ? CDCSuspensionParams::Rd_MR_f : CDCSuspensionParams::Rd_MR_r;
        const double Ap_MR = front_rear ? CDCSuspensionParams::Ap_MR_f : CDCSuspensionParams::Ap_MR_r;
        const double Td_MR = front_rear ? CDCSuspensionParams::Td_MR_f : CDCSuspensionParams::Td_MR_r;
        const double C0 = front_rear ? CDCSuspensionParams::C0_f : CDCSuspensionParams::C0_r;
        const double C1 = front_rear ? CDCSuspensionParams::C1_f : CDCSuspensionParams::C1_r;
        const double C2 = front_rear ? CDCSuspensionParams::C2_f : CDCSuspensionParams::C2_r;
        const double C3 = front_rear ? CDCSuspensionParams::C3_f : CDCSuspensionParams::C3_r;

        // 计算磁流变剪切应力 Tau_MR
        const double H_MR = (2 * suspension_current[i]) / (2 * Td_MR); // 磁场强度近似
        const double Tau_MR = C0 + C1 * H_MR + C2 * std::pow(H_MR, 2) + C3 * std::pow(H_MR, 3);

        // 基础粘性阻尼系数
        const double Cvis_MR = (12 * Eta_MR * L_MR * std::pow(Ap_MR, 2)) / (PI * Rd_MR * std::pow(Td_MR, 3));

        // 速度方向与死区判断
        double velocity = suspension_dx_s[i];
        double sign_v = 0.0;
        if (velocity > 0.001) sign_v = 1.0;
        else if (velocity < -0.001) sign_v = -1.0;

        // 基础阻尼力
        F_c[i] = Cvis_MR * velocity;

        // 磁流变可控力
        double MR_Force_Base = ((Ap_MR * 3 * L_MR * Tau_MR) / Td_MR) + (PI * Rd_MR * L_MR * Tau_MR);
        F_MR[i] = MR_Force_Base * sign_v;

        F_total[i] = F_c[i] + F_MR[i];
    }
    return F_total;
}