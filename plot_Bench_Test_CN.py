import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np

# ==============================================================================
# 0. Matplotlib 中文显示配置 (核心步骤)
# ==============================================================================
# 兼容 Windows 和 macOS 的中文字体设置
plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'Arial Unicode MS', 'sans-serif']
plt.rcParams['axes.unicode_minus'] = False  # 确保负号正常显示

# ==============================================================================
# 1. 读取数据
# ==============================================================================
csv_file = "active_chassis_data.csv"
try:
    df = pd.read_csv(csv_file, skipinitialspace=True)
except FileNotFoundError:
    print(f"错误：找不到 {csv_file}。请检查路径。")
    exit()

# 提取物理量
time = df["Time"]
displacement_mm = df["ActualHeight_FL"] * 1000  # m 转换为 mm (悬架行程)
force_air_N = df["AirForce_FL"]                 # N (空气弹簧力)
stiffness_kN_m = df["Stiffness_FL"] / 1000      # N/m 转换为 kN/m (瞬时刚度)
damping_Ns_m = df["AirDamping_FL"]              # Ns/m (热力学阻尼)

# ==============================================================================
# 2. 绘制四大特性图 (中文版)
# ==============================================================================
fig = plt.figure(figsize=(16, 12))
gs = gridspec.GridSpec(2, 2)
plt.suptitle('单腔空气弹簧：台架测试力学特性验证', fontsize=22, weight='bold')

# ------------------------------------------------------------------------------
# 图 1: 迟滞回线 (力-位移特性)
# ------------------------------------------------------------------------------
ax1 = fig.add_subplot(gs[0, 0])
# 颜色渐变表示时间轨迹
sc = ax1.scatter(displacement_mm, force_air_N, c=np.linspace(0, 1, len(df)), cmap='Oranges_r', s=5, alpha=0.8)

ax1.set_title('(1) 迟滞回线 (力-位移特性)', fontsize=15, fontweight='bold')
ax1.set_xlabel('悬架行程 (mm)\n[<-- 压缩 | 拉伸 -->]', fontsize=13)
ax1.set_ylabel('空气弹簧力 (N)', fontsize=13)
ax1.grid(True, linestyle='--', alpha=0.5)

# 添加带背景框的中文注释
bbox_props = dict(boxstyle="square,pad=0.3", fc="white", ec="black", alpha=0.8)
ax1.text(displacement_mm.mean(), force_air_N.mean() + 100, "面积 = 能量损耗\n(热力学阻尼效应)",
         ha='center', va='center', bbox=bbox_props, fontsize=11)

# ------------------------------------------------------------------------------
# 图 2: 非线性刚度特性
# ------------------------------------------------------------------------------
ax2 = fig.add_subplot(gs[0, 1])
ax2.plot(displacement_mm, stiffness_kN_m, 'r-', linewidth=2.5)

ax2.set_title('(2) 非线性刚度特性 ($k_1$)', fontsize=15, fontweight='bold')
ax2.set_xlabel('悬架行程 (mm)', fontsize=13)
ax2.set_ylabel('瞬时刚度 (kN/m)', fontsize=13)
ax2.grid(True, linestyle='--', alpha=0.5)

# ------------------------------------------------------------------------------
# 图 3: 热力学阻尼特性
# ------------------------------------------------------------------------------
ax3 = fig.add_subplot(gs[1, 0])
ax3.plot(displacement_mm, damping_Ns_m, 'g-', linewidth=2.5)

ax3.set_title('(3) 热力学阻尼特性 ($c_1$)', fontsize=15, fontweight='bold')
ax3.set_xlabel('悬架行程 (mm)', fontsize=13)
ax3.set_ylabel('阻尼系数 (Ns/m)', fontsize=13)
ax3.grid(True, linestyle='--', alpha=0.5)

# ------------------------------------------------------------------------------
# 图 4: 动态时域响应
# ------------------------------------------------------------------------------
ax4 = fig.add_subplot(gs[1, 1])

# 画位移 (左侧 Y 轴)
line1 = ax4.plot(time, displacement_mm, 'b--', linewidth=1.5, label='位移 (mm)')
ax4.set_ylabel('位移 (mm)', color='blue', fontsize=13)
ax4.tick_params(axis='y', labelcolor='blue')
ax4.set_xlabel('时间 (s)', fontsize=13)

# 画力 (右侧 Y 轴)
ax4_twin = ax4.twinx()
line2 = ax4_twin.plot(time, force_air_N, 'k-', linewidth=1.5, label='支撑力 (N)')
ax4_twin.set_ylabel('支撑力 (N)', color='black', fontsize=13)
ax4_twin.tick_params(axis='y', labelcolor='black')

ax4.set_title('(4) 动态时域响应', fontsize=15, fontweight='bold')
ax4.grid(True, linestyle='--', alpha=0.5)

plt.tight_layout()
plt.show()