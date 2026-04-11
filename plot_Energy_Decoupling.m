% =========================================================================
% 单腔空气弹簧：动力学相平面与内部能量解耦分析 (单图独立生成版)
% =========================================================================
clc; clear; close all;

% 1. 读取 CSV 数据
data = readtable('active_chassis_data.csv');

% 2. 截取稳态数据 (例如 3.0 秒以后的数据，避免重叠与瞬态干扰)
idx = data.Time >= 3.0;
time = data.Time(idx);
disp_m = data.ActualHeight_FL(idx);
disp_mm = disp_m * 1000;
vel_ms = data.BodyVel(idx);
f_total = data.AirForce_FL(idx);
c1 = data.AirDamping_FL(idx);

% 3. 核心物理量解耦计算
% (1) 计算纯热力学阻尼力 (F_damping = c1 * v)
f_damping = c1 .* vel_ms;
% (2) 计算纯气压弹性力 (F_elastic = F_total - F_damping)
f_elastic = f_total - f_damping;
% (3) 计算累积耗散能量 (积分 W = ∫ F dx)
% 使用离散差分计算位移增量，首项补0保持数组长度一致
dx_m = [0; diff(disp_m)]; 
cumulative_energy = cumsum(f_total .* dx_m); % 累积做功 (焦耳)

% 4. 基础页面与字体设置
fs = 22;  % 统一字体大小
lw = 2.5; % 统一线宽
% 设置单张图片的默认弹出尺寸 [左下角x, 左下角y, 宽度, 高度]
fig_pos = [200, 200, 800, 600]; 

% =========================================================================
% 图 1: 动力学相平面图 (Phase Portrait)
% =========================================================================
figure(1);
set(gcf, 'position', fig_pos, 'color', 'white');
plot(disp_mm, vel_ms, 'b-', 'LineWidth', lw);
set(gca, 'FontSize', fs);
xlabel('悬架动行程 [mm]', 'fontsize', fs);
ylabel('悬架垂向速度 [m/s]', 'fontsize', fs);
title('系统相平面轨迹 (极限环稳定性)', 'fontsize', fs);
grid on; hold on;
% 添加零线十字坐标轴
plot(xlim, [0 0], 'k-', 'LineWidth', 1);
plot([0 0], ylim, 'k-', 'LineWidth', 1);

% =========================================================================
% 图 2: 内部受力解耦 (Force Decomposition)
% =========================================================================
figure(2);
set(gcf, 'position', fig_pos + [40, -40, 0, 0], 'color', 'white');
plot(disp_mm, f_total, 'Color', [0.6 0.6 0.6], 'LineWidth', 1.5); hold on;
plot(disp_mm, f_elastic, 'r-', 'LineWidth', lw);
plot(disp_mm, f_damping, 'g--', 'LineWidth', lw);
set(gca, 'FontSize', fs);
xlabel('悬架动行程 [mm]', 'fontsize', fs);
ylabel('作用力 [N]', 'fontsize', fs);
title(' 弹性力与热力学阻尼力解耦', 'fontsize', fs);
% 图例靠左放置，并去掉黑框
legend({'总输出力', '纯弹性力', '纯阻尼力'}, ...
    'fontsize', 18, 'Location', 'west', 'Box', 'off');
grid on;

% =========================================================================
% 图 3: 热力学阻尼外特性 (Damping Force vs Velocity)
% =========================================================================
figure(3);
set(gcf, 'position', fig_pos + [80, -80, 0, 0], 'color', 'white');
% 使用散点图并映射颜色
scatter(vel_ms, f_damping, 30, disp_mm, 'filled');
colormap(gca, 'jet'); % 经典热力图配色
cb = colorbar;
cb.Label.String = '悬架行程 [mm]';
cb.Label.FontSize = 18;
set(gca, 'FontSize', fs);
xlabel('悬架垂向速度 [m/s]', 'fontsize', fs);
ylabel('热力学阻尼力 [N]', 'fontsize', fs);
title(' 热力学阻尼外特性 (F_d-v 状态依赖)', 'fontsize', fs);
grid on; 

% =========================================================================
% 图 4: 累积做功与机械能耗散 (Energy Dissipation)
% =========================================================================
figure(4);
set(gcf, 'position', fig_pos + [120, -120, 0, 0], 'color', 'white');
plot(time, cumulative_energy, 'Color', [0.5 0 0.5], 'LineWidth', lw); % 紫色曲线
set(gca, 'FontSize', fs);
xlabel('时间 [s]', 'fontsize', fs);
ylabel('累积耗散能 / 做功 [Joule]', 'fontsize', fs);
title(' 循环做功与累积能量耗散', 'fontsize', fs);
grid on;