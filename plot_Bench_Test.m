% =========================================================================
% 单腔空气弹簧：台架测试力学特性验证 (单图独立生成版)
% =========================================================================
clc; clear; close all;

% 1. 读取 CSV 数据
data = readtable('active_chassis_data.csv');

% 提取完整时域数据 (用于图4，展示完整的动态跟随过程)
t = data.Time;
disp_mm = data.ActualHeight_FL * 1000;      % 悬架行程 [mm]
force_N = data.AirForce_FL;                 % 弹簧力 [N]

% 提取稳态周期数据 (用于图1、2、3，只取最后1秒，防止迟滞回线重叠)
idx = data.Time >= 4.0;
disp_mm_steady = data.ActualHeight_FL(idx) * 1000;
force_N_steady = data.AirForce_FL(idx);
stiffness_kNm_steady = data.Stiffness_FL(idx) / 1000;
damping_Nsm_steady = data.AirDamping_FL(idx);

% 2. 基础页面与字体设置
fs = 22;  % 字体大小
lw = 2.5; % 线宽
% 设置单张图片的默认弹出尺寸 [左下角x, 左下角y, 宽度, 高度]
fig_pos = [200, 200, 800, 600]; 

% =========================================================================
% 图 1: 迟滞回线 (力-位移特性)
% =========================================================================
figure(1);
set(gcf, 'position', fig_pos, 'color', 'white');
plot(disp_mm_steady, force_N_steady, 'Color', [0.850 0.325 0.098], 'LineWidth', lw); 
set(gca, 'FontSize', fs);
xlabel('悬架行程 [mm] [<-- 压缩 | 拉伸 -->]', 'fontsize', fs);
ylabel('空气弹簧力 [N]', 'fontsize', fs);
title(' 迟滞回线 (力-位移特性)', 'fontsize', fs);
grid on; 

% =========================================================================
% 图 2: 非线性刚度特性
% =========================================================================
figure(2);
set(gcf, 'position', fig_pos + [40, -40, 0, 0], 'color', 'white'); % 窗口稍微错开方便查看
plot(disp_mm_steady, stiffness_kNm_steady, 'r-', 'LineWidth', lw);
set(gca, 'FontSize', fs);
xlabel('悬架行程 [mm]', 'fontsize', fs);
ylabel('瞬时刚度 [kN/m]', 'fontsize', fs);
title(' 非线性刚度特性 (k_1)', 'fontsize', fs);
grid on; 

% =========================================================================
% 图 3: 热力学阻尼特性
% =========================================================================
figure(3);
set(gcf, 'position', fig_pos + [80, -80, 0, 0], 'color', 'white');
plot(disp_mm_steady, damping_Nsm_steady, 'g-', 'LineWidth', lw);
set(gca, 'FontSize', fs);
xlabel('悬架行程 [mm]', 'fontsize', fs);
ylabel('阻尼系数 [Ns/m]', 'fontsize', fs);
title(' 热力学阻尼特性 (c_1)', 'fontsize', fs);
grid on; 

% =========================================================================
% 图 4: 动态时域响应
% =========================================================================
figure(4);
set(gcf, 'position', fig_pos + [120, -120, 0, 0], 'color', 'white');
[hAx, hLine1, hLine2] = plotyy(t, disp_mm, t, force_N);

set(hLine1, 'LineStyle', '--', 'Color', 'b', 'LineWidth', 2);
set(hLine2, 'LineStyle', '-', 'Color', 'k', 'LineWidth', 2);

set(hAx(1), 'ycolor', 'b', 'FontSize', fs);
set(hAx(2), 'ycolor', 'k', 'FontSize', fs);

xlabel('时间 [s]', 'fontsize', fs);
ylabel(hAx(1), '位移 [mm]', 'fontsize', fs);
ylabel(hAx(2), '支撑力 [N]', 'fontsize', fs);
title(' 动态时域响应', 'fontsize', fs);

legend([hLine1, hLine2], {'位移 [mm]', '支撑力 [N]'}, 'fontsize', 18, 'Location', 'northeast', 'Box', 'off');
grid on;