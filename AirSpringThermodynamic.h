#ifndef AIR_SPRING_THERMODYNAMIC_H
#define AIR_SPRING_THERMODYNAMIC_H
#include <array>

constexpr double PI = 3.14159265358979323846;

// =========================================================================
// 组件 1: CDC 减震器 (CDCDamper)
// =========================================================================
class CDCDamper {
public:
    struct Params {
        static double Eta_MR_f, Eta_MR_r;
        static double L_MR_f, L_MR_r;
        static double Rd_MR_f, Rd_MR_r;
        static double Ap_MR_f, Ap_MR_r;
        static double Td_MR_f, Td_MR_r;
        static double C0_f, C0_r;
        static double C1_f, C1_r;
        static double C2_f, C2_r;
        static double C3_f, C3_r;
    };

    // 计算 CDC 阻尼力
    std::array<double, 4> calculate_force(
            const std::array<double, 4> &current_cmd,
            const std::array<double, 4> &suspension_dx_s);
};

// =========================================================================
// 组件 2: 单腔空气弹簧 (SingleChamberAirSpring)
//    管理气量、迟滞状态、计算刚度力
// 以后如果有双腔，就再写一个 DualChamberAirSpring 类
// =========================================================================
class SingleChamberAirSpring {
public:
    struct Params {
        static double P_b0_f, P_b0_r;//前后轴初始绝对压力
        static double V_b0_f, V_b0_r;//
        static double T_atm;
        static double Kb_f, Kb_r;
        static double Cv, R_gas, Gamma;
        static double A_eff_slope_f, A_eff_slope_r;
        static double A_eff_int_f, A_eff_int_r;
        static double H0_f, H0_r;
    };
    std::array<double, 4> zm_state = {0.0, 0.0, 0.0, 0.0};        // 迟滞状态
    std::array<double, 4> current_gas_mass = {0.0, 0.0, 0.0, 0.0}; // 气量状态
    bool is_initialized = false;
    // --- 调试接口 ---
    double debug_k1_FL = 0.0;
    double debug_c1_FL = 0.0;
    // 计算空气弹簧力
    std::array<double, 4> calculate_force(
            const std::array<double, 4> &flow_rate_cmd,
            const std::array<double, 4> &x_s,
            const std::array<double, 4> &dx_s,
            double dt);
};


// =========================================================================
// 总成: 悬架系统 (SuspensionSystem)
// 职责：把零部件组装起来
// =========================================================================
class SuspensionSystem {
public:
    // 拥有两个组件对象
    SingleChamberAirSpring airSpring; // 单腔组件
    CDCDamper cdcDamper;              // CDC组件
    // 以后可以很方便地扩展，例如：
    // DualChamberAirSpring airSpringDual;
};

#endif // AIR_SPRING_THERMODYNAMIC_H