#include "AirSpringThermodynamic.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

// --- 单腔空气弹簧参数 ---
double SingleChamberAirSpring::Params::P_b0_f = 5.1e5;
double SingleChamberAirSpring::Params::P_b0_r = 5.1e5;
double SingleChamberAirSpring::Params::V_b0_f = 2.45e-3;
double SingleChamberAirSpring::Params::V_b0_r = 2.45e-3;
double SingleChamberAirSpring::Params::T_atm = 298.0;
double SingleChamberAirSpring::Params::R_gas = 287.0;
double SingleChamberAirSpring::Params::Gamma = 1.4;
double SingleChamberAirSpring::Params::Cv = 717.5;
double SingleChamberAirSpring::Params::Kb_f = 135.0;
double SingleChamberAirSpring::Params::Kb_r = 135.0;
double SingleChamberAirSpring::Params::A_eff_slope_f = -0.009545;
double SingleChamberAirSpring::Params::A_eff_slope_r = -0.009545;
double SingleChamberAirSpring::Params::A_eff_int_f = 0.013;
double SingleChamberAirSpring::Params::A_eff_int_r = 0.013;
double SingleChamberAirSpring::Params::H0_f = 0.23;
double SingleChamberAirSpring::Params::H0_r = 0.23;
// --- CDC 减震器参数 ---
double CDCDamper::Params::Eta_MR_f = 0.0197;
double CDCDamper::Params::Eta_MR_r = 0.0197;
double CDCDamper::Params::L_MR_f = 0.4;
double CDCDamper::Params::L_MR_r = 0.4;
double CDCDamper::Params::Rd_MR_f = 0.076;
double CDCDamper::Params::Rd_MR_r = 0.076;
double CDCDamper::Params::Ap_MR_f = 0.00226708;
double CDCDamper::Params::Ap_MR_r = 0.00226708;
double CDCDamper::Params::Td_MR_f = 0.001;
double CDCDamper::Params::Td_MR_r = 0.001;
double CDCDamper::Params::C0_f = 0.1;
double CDCDamper::Params::C0_r = 0.1;
double CDCDamper::Params::C1_f = 0.15;
double CDCDamper::Params::C1_r = 0.15;
double CDCDamper::Params::C2_f = 0.000116;
double CDCDamper::Params::C2_r = 0.000116;
double CDCDamper::Params::C3_f = 0.000000105;
double CDCDamper::Params::C3_r = 0.000000105;

// =========================================================================
// 单腔空气弹簧实现
// =========================================================================
std::array<double, 4> SingleChamberAirSpring::calculate_force(
        const std::array<double, 4> &control_flow_rate,
        const std::array<double, 4> &suspension_x_s,
        const std::array<double, 4> &suspension_dx_s,
        double dt) {

    std::array<double, 4> F_total{};
    const double P_atm = 101325.0;

    // 1. 初始化 (使用 this-> 访问组件内部状态)
    if (!is_initialized) {
        double m0_f = (Params::P_b0_f * Params::V_b0_f) / (Params::R_gas * Params::T_atm);
        double m0_r = (Params::P_b0_r * Params::V_b0_r) / (Params::R_gas * Params::T_atm);
        current_gas_mass = {m0_f, m0_f, m0_r, m0_r};
        is_initialized = true;
    }

    for (size_t i = 0; i < 4; ++i) {
        const bool front_rear = (i < 2);
        // 参数引用前缀改为 Params::
        const double V_b0_geom = front_rear ? Params::V_b0_f : Params::V_b0_r;
        const double Kb = front_rear ? Params::Kb_f : Params::Kb_r;
        const double Slope_A = front_rear ? Params::A_eff_slope_f : Params::A_eff_slope_r;
        const double Int_A = front_rear ? Params::A_eff_int_f : Params::A_eff_int_r;
        const double H0 = front_rear ? Params::H0_f : Params::H0_r;

        // 更新质量 (访问成员变量)
        this->current_gas_mass[i] += control_flow_rate[i] * dt;
        if (this->current_gas_mass[i] < 1e-6) this->current_gas_mass[i] = 1e-6;

        const double P_active = (this->current_gas_mass[i] * Params::R_gas * Params::T_atm) / V_b0_geom;

        const double Current_H = H0 - suspension_x_s[i];
        double A_eff = Slope_A * Current_H + Int_A;
        if (A_eff < 1e-5) A_eff = 1e-5;

        double Current_V_b = V_b0_geom - A_eff * suspension_x_s[i];
        if (Current_V_b < 1e-6) Current_V_b = 1e-6;

        double Safe_P_b = P_active * std::pow(V_b0_geom / Current_V_b, Params::Gamma);
        F_total[i] = (Safe_P_b - P_atm) * A_eff;

        // 计算阻尼供调试
        const double K1 = (Params::Gamma * Safe_P_b * std::pow(A_eff, 2)) / Current_V_b;
        double C1 = 0.0;
        if (Kb > 1e-9) C1 = ((Params::Cv * this->current_gas_mass[i]) / Kb) * K1;

        if (i == 0) {
            double K_pressure = (Params::Gamma * Safe_P_b * A_eff * A_eff) / Current_V_b;
            double K_area = (Safe_P_b - P_atm) * (-Slope_A);
            this->debug_k1_FL = K_pressure + K_area;
            this->debug_c1_FL = C1; // 记录阻尼
        }
    }
    return F_total;
}

// =========================================================================
// CDC 减震器实现
// =========================================================================
std::array<double, 4> CDCDamper::calculate_force(
        const std::array<double, 4> &current_cmd,
        const std::array<double, 4> &suspension_dx_s) {

    std::array<double, 4> F_c{}, F_MR{}, F_total{};

    for (size_t i = 0; i < 4; ++i) {
        const bool front_rear = (i < 2);
        // 参数引用前缀改为 Params::
        const double Eta_MR = front_rear ? Params::Eta_MR_f : Params::Eta_MR_r;
        const double L_MR = front_rear ? Params::L_MR_f : Params::L_MR_r;
        const double Rd_MR = front_rear ? Params::Rd_MR_f : Params::Rd_MR_r;
        const double Ap_MR = front_rear ? Params::Ap_MR_f : Params::Ap_MR_r;
        const double Td_MR = front_rear ? Params::Td_MR_f : Params::Td_MR_r;
        const double C0 = front_rear ? Params::C0_f : Params::C0_r;
        const double C1 = front_rear ? Params::C1_f : Params::C1_r;
        const double C2 = front_rear ? Params::C2_f : Params::C2_r;
        const double C3 = front_rear ? Params::C3_f : Params::C3_r;

        const double H_MR = (2 * current_cmd[i]) / (2 * Td_MR);
        const double Tau_MR = C0 + C1 * H_MR + C2 * std::pow(H_MR, 2) + C3 * std::pow(H_MR, 3);
        const double Cvis_MR = (12 * Eta_MR * L_MR * std::pow(Ap_MR, 2)) / (PI * Rd_MR * std::pow(Td_MR, 3));

        double velocity = suspension_dx_s[i];
        double sign_v = (velocity > 0.001) ? 1.0 : ((velocity < -0.001) ? -1.0 : 0.0);

        F_c[i] = Cvis_MR * velocity;
        double MR_Force_Base = ((Ap_MR * 3 * L_MR * Tau_MR) / Td_MR) + (PI * Rd_MR * L_MR * Tau_MR);
        F_MR[i] = MR_Force_Base * sign_v;
        F_total[i] = F_c[i] + F_MR[i];
    }
    return F_total;
}