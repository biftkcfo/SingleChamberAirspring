#ifndef AIR_SPRING_THERMODYNAMIC_H
#define AIR_SPRING_THERMODYNAMIC_H
#include <array>
#ifndef pi
#define pi 3.14159265358979323846
#endif
class Suspension {
public:
    // 1. 参数结构体 (按照规范：静态、区分前后轴)
    struct AirSpringParams {
        // --- 初始物理状态 ---
        static double P_b0_f;      // 前轴初始绝对压力 (Pa)
        static double P_b0_r;      // 后轴初始绝对压力 (Pa)
        static double V_b0_f;      // 前轴初始体积 (m^3)
        static double V_b0_r;      // 后轴初始体积 (m^3)
        static double T_atm;       // 环境温度 (K)
        // --- 热力学系数 ---
        static double Kb_f;        // 前轴热交换系数 (决定迟滞大小)
        static double Kb_r;        // 后轴热交换系数
        static double Cv;          // 定容比热容 (J/kg*K)
        static double R_gas;       // 气体常数
        static double Gamma;       // 绝热指数
        // --- 有效面积几何参数 (A_eff = Slope * h + Intercept) ---
        static double A_eff_slope_f;
        static double A_eff_slope_r;
        static double A_eff_int_f; // Intercept
        static double A_eff_int_r;
        // --- 初始高度 ---
        static double H0_f;
        static double H0_r;
    };
    //CDC减震器参数（CDC Damper Parameters）
    struct CDCSuspensionParams {
        static double Eta_MR_f;
        static double Eta_MR_r;
        static double L_MR_f;
        static double L_MR_r;
        static double Rd_MR_f;
        static double Rd_MR_r;
        static double Ap_MR_f;
        static double Ap_MR_r;
        static double Td_MR_f;
        static double Td_MR_r;
        static double C0_f;//磁流变多项式系数
        static double C0_r;
        static double C1_f;
        static double C1_r;
        static double C2_f;
        static double C2_r;
        static double C3_f;
        static double C3_r;

        // 记录左前轮(FL)的实时状态，供 main 函数读取保存
        double debug_k1_FL = 0.0; // 实时气动刚度
        double debug_c1_FL = 0.0; // 实时空气阻尼
    };

    std::array<double, 4> zm_state = {0.0, 0.0, 0.0, 0.0};       // 空气弹簧迟滞状态
    std::array<double, 4> current_gas_mass = {0.0, 0.0, 0.0, 0.0};//实时气体质量
    bool is_initialized = false;
    // （新增）用于数据记录的调试变量 (左前轮)
    double debug_k1_FL = 0.0;
    double debug_c1_FL = 0.0;
    // 4. 功能接口
    /**
     * @brief 计算主动空气弹簧力 (含充放气控制)
     * @param control_flow_rate 充放气质量流率 (kg/s) [慢速环输入]
     * @param suspension_x_s 悬架动行程 （m)
     * @param suspension_dx_s 悬架动速度 （m/s）
     * @param dt 时间步长 (s)
     */
    std::array<double, 4> F_active_air_spring(
            const std::array<double, 4> &control_flow_rate,
            const std::array<double, 4> &suspension_x_s,
            const std::array<double, 4> &suspension_dx_s,
            const double &dt);

    /**
     * @brief 计算 CDC 减震器力 (基于实验室规范)
     * @param suspension_current 控制电流 (A) [快速环输入]
     * @param suspension_dx_s 悬架动速度（m/s）
     */
    std::array<double, 4> F_cdc_damper(
            const std::array<double, 4> &suspension_current,
            const std::array<double, 4> &suspension_dx_s);
};

#endif