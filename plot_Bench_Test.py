import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np

# ==============================================================================
# 1. 读取数据
# ==============================================================================
csv_file = "active_chassis_data.csv"
try:
    df = pd.read_csv(csv_file, skipinitialspace=True)
except FileNotFoundError:
    print(f"错误：找不到 {csv_file}。请先运行 C++ 程序生成数据。")
    exit()

# 提取物理量
# 注意：在 main.cpp 改完后，ActualHeight_FL 应该就是我们输入的正弦波
displacement_mm = df["ActualHeight_FL"] * 1000  # mm (行程)
force_air_N = df["AirForce_FL"]                 # N (空气弹簧力)
stiffness_kN_m = df["Stiffness_FL"] / 1000      # kN/m (瞬时刚度)
damping_Ns_m = df["AirDamping_FL"]              # Ns/m (热力学阻尼)

# ==============================================================================
# 2. 绘制四大特性图 (Bench Test Analysis)
# ==============================================================================
fig = plt.figure(figsize=(16, 12))
gs = gridspec.GridSpec(2, 2)
plt.suptitle('Single Chamber Air Spring: Bench Test Characteristics', fontsize=20, weight='bold')

# ------------------------------------------------------------------------------
# 图 1: 示功图 (Force vs Displacement) - 最重要的特性图
# 含义：展示刚度(斜率)和迟滞耗能(面积)
# ------------------------------------------------------------------------------
ax1 = fig.add_subplot(gs[0, 0])
# 使用散点图并用颜色表示过程（压缩/拉伸），方便看轨迹方向
sc = ax1.scatter(displacement_mm, force_air_N, c=np.linspace(0, 1, len(df)), cmap='jet', s=3, alpha=0.5)

ax1.set_title('(1) Hysteresis Loop (Force vs Displacement)', fontsize=14, fontweight='bold')
ax1.set_xlabel('Suspension Deflection (mm)\n[<-- Compression | Extension -->]', fontsize=12)
ax1.set_ylabel('Air Spring Force (N)', fontsize=12)
ax1.grid(True, linestyle='--', alpha=0.5)

# 添加说明
ax1.text(displacement_mm.mean(), force_air_N.mean(), "Area = Energy Loss\n(Thermodynamic Damping)",
         ha='center', va='center', bbox=dict(facecolor='white', alpha=0.8))

# ------------------------------------------------------------------------------
# 图 2: 刚度特性 (Stiffness vs Displacement)
# 含义：展示几何非线性 (Geometric Nonlinearity)
# ------------------------------------------------------------------------------
ax2 = fig.add_subplot(gs[0, 1])
ax2.plot(displacement_mm, stiffness_kN_m, 'r-', linewidth=2.0)

ax2.set_title('(2) Stiffness Characteristic (k1)', fontsize=14, fontweight='bold')
ax2.set_xlabel('Suspension Deflection (mm)', fontsize=12)
ax2.set_ylabel('Instant Stiffness (kN/m)', fontsize=12)
ax2.grid(True, linestyle='--', alpha=0.5)

# ------------------------------------------------------------------------------
# 图 3: 阻尼特性 (Damping vs Displacement)
# 含义：展示热力学阻尼随行程的变化
# ------------------------------------------------------------------------------
ax3 = fig.add_subplot(gs[1, 0])
ax3.plot(displacement_mm, damping_Ns_m, 'g-', linewidth=2.0)

ax3.set_title('(3) Thermodynamic Damping (c1)', fontsize=14, fontweight='bold')
ax3.set_xlabel('Suspension Deflection (mm)', fontsize=12)
ax3.set_ylabel('Damping Coefficient (Ns/m)', fontsize=12)
ax3.grid(True, linestyle='--', alpha=0.5)

# ------------------------------------------------------------------------------
# 图 4: 动态响应时序 (Time Series)
# 含义：检查输入输出是否正常
# ------------------------------------------------------------------------------
ax4 = fig.add_subplot(gs[1, 1])
time = df["Time"]
ax4.plot(time, displacement_mm, 'b--', label='Displacement (mm)')
ax4.set_ylabel('Disp (mm)', color='blue')
ax4.tick_params(axis='y', labelcolor='blue')

ax4_twin = ax4.twinx()
ax4_twin.plot(time, force_air_N, 'k-', label='Force (N)')
ax4_twin.set_ylabel('Force (N)', color='black')

ax4.set_title('(4) Dynamic Response (Time Domain)', fontsize=14, fontweight='bold')
ax4.set_xlabel('Time (s)', fontsize=12)
ax4.grid(True, linestyle='--', alpha=0.5)

plt.tight_layout()
plt.show()