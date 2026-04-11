% =========================================================================
% CDC 磁流变减震器：物理特性解析与力学建模分析
% =========================================================================
clc; clear; close all;

% 1. 导入您的 C++ 模型参数 (CDCSuspensionParams)
Eta_MR = 0.0197; L_MR = 0.4; Rd_MR = 0.076; Ap_MR = 0.00226708; Td_MR = 0.001;
C0 = 0.1; C1 = 0.15; C2 = 0.000116; C3 = 0.000000105;

% 计算基础粘性阻尼系数
Cvis_MR = (12 * Eta_MR * L_MR * Ap_MR^2) / (pi * Rd_MR * Td_MR^3);

% 2. 构造标准台架激励 (1Hz, ±30mm)
t = 0:0.001:1; % 单个完整周期 1秒
disp_m = 0.03 * sin(2*pi*t);
vel_ms = 0.03 * 2*pi * cos(2*pi*t);

% 为了避免符号函数 sign(v) 在0点突变导致画图不平滑，使用与您C++相近的过渡死区
sign_v = zeros(size(vel_ms));
sign_v(vel_ms > 0.001) = 1;
sign_v(vel_ms < -0.001) = -1;
idx = (vel_ms >= -0.001 & vel_ms <= 0.001);
sign_v(idx) = vel_ms(idx) / 0.001; % 线性过渡

% 3. 测试电流矩阵
I_list = [0, 0.5, 1.0, 1.5, 2.0];
colors = lines(length(I_list)); % 生成不同颜色

% --- 全局排版设置 ---
fs = 22; lw = 2.5;
fig_pos = [200, 200, 800, 600];

% =========================================================================
% 图 1: CDC 力-速度外特性 (F_d-v 曲线) - 减震器最重要的图
% =========================================================================
figure(1); set(gcf, 'position', fig_pos, 'color', 'white'); hold on; grid on;
for i = 1:length(I_list)
    I = I_list(i);
    H_MR = (2 * I) / (2 * Td_MR);
    Tau_MR = C0 + C1*H_MR + C2*(H_MR^2) + C3*(H_MR^3);
    MR_Force_Base = ((Ap_MR * 3 * L_MR * Tau_MR) / Td_MR) + (pi * Rd_MR * L_MR * Tau_MR);
    F_total = Cvis_MR * vel_ms + MR_Force_Base * sign_v;
    plot(vel_ms, F_total, 'Color', colors(i,:), 'LineWidth', lw, 'DisplayName', sprintf('I = %.1f A', I));
end
set(gca, 'FontSize', fs);
xlabel('悬架垂向速度 [m/s]', 'fontsize', fs); ylabel('CDC 阻尼力 [N]', 'fontsize', fs);
title(' CDC 阻尼力-速度外特性 (F_d-v)', 'fontsize', fs);
legend('fontsize', 18, 'Location', 'northwest', 'Box', 'off');

% =========================================================================
% 图 2: CDC 力-位移示功图 (F_d-s 曲线)
% =========================================================================
figure(2); set(gcf, 'position', fig_pos + [40, -40, 0, 0], 'color', 'white'); hold on; grid on;
for i = 1:length(I_list)
    I = I_list(i);
    H_MR = (2 * I) / (2 * Td_MR);
    Tau_MR = C0 + C1*H_MR + C2*(H_MR^2) + C3*(H_MR^3);
    MR_Force_Base = ((Ap_MR * 3 * L_MR * Tau_MR) / Td_MR) + (pi * Rd_MR * L_MR * Tau_MR);
    F_total = Cvis_MR * vel_ms + MR_Force_Base * sign_v;
    plot(disp_m*1000, F_total, 'Color', colors(i,:), 'LineWidth', lw, 'DisplayName', sprintf('I = %.1f A', I));
end
set(gca, 'FontSize', fs);
xlabel('悬架动行程 [mm]', 'fontsize', fs); ylabel('CDC 阻尼力 [N]', 'fontsize', fs);
title(' CDC 阻尼力-位移示功图 (F_d-s)', 'fontsize', fs);
legend('fontsize', 18, 'Location', 'southeast', 'Box', 'off');

% =========================================================================
% 图 3: 磁流变液剪切屈服非线性特性 (Tau-I 曲线)
% =========================================================================
figure(3); set(gcf, 'position', fig_pos + [80, -80, 0, 0], 'color', 'white'); hold on; grid on;
I_range = 0:0.01:2.5;
H_range = (2 * I_range) / (2 * Td_MR);
Tau_range = C0 + C1.*H_range + C2.*(H_range.^2) + C3.*(H_range.^3);
plot(I_range, Tau_range, 'Color', [0.5 0 0.5], 'LineWidth', lw);
set(gca, 'FontSize', fs);
xlabel('控制电流 [A]', 'fontsize', fs); ylabel('剪切屈服应力 \tau_{MR} [kPa]', 'fontsize', fs);
title('磁流变液剪切屈服特性', 'fontsize', fs);

% =========================================================================
% 图 4: CDC 时域动态响应波形 (F_d-t 曲线)
% =========================================================================
figure(4); set(gcf, 'position', fig_pos + [120, -120, 0, 0], 'color', 'white'); hold on; grid on;
for i = 1:length(I_list)
    I = I_list(i);
    H_MR = (2 * I) / (2 * Td_MR);
    Tau_MR = C0 + C1*H_MR + C2*(H_MR^2) + C3*(H_MR^3);
    MR_Force_Base = ((Ap_MR * 3 * L_MR * Tau_MR) / Td_MR) + (pi * Rd_MR * L_MR * Tau_MR);
    F_total = Cvis_MR * vel_ms + MR_Force_Base * sign_v;
    plot(t, F_total, 'Color', colors(i,:), 'LineWidth', lw, 'DisplayName', sprintf('I = %.1f A', I));
end
set(gca, 'FontSize', fs);
xlabel('时间 [s]', 'fontsize', fs); ylabel('CDC 阻尼力 [N]', 'fontsize', fs);
title(' CDC 动态阻尼力时域响应', 'fontsize', fs);
legend('fontsize', 14, 'Location', 'northeast', 'Box', 'off', 'NumColumns', 2);