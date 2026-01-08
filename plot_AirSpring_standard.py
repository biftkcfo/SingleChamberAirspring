import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

# 1. 读取数据
csv_file = "active_chassis_data.csv"
try:
    df = pd.read_csv(csv_file, skipinitialspace=True)
except FileNotFoundError:
    print(f"错误：找不到 {csv_file}。请确保已运行修改后的 C++ 程序。")
    exit()

# 2. 提取数据列
time = df["Time"]
speed = df["Speed_kmh"]
height_mm = df["ActualHeight_FL"] * 1000  # m -> mm
stiffness_kN_m = df["Stiffness_FL"] / 1000 # N/m -> kN/m
air_damping = df["AirDamping_FL"]          # Ns/m
cdc_current = df["Current_FL"]             # A
flow_rate_g_s = df["FlowRate_FL"] * 1000   # kg/s -> g/s (克/秒)

# 3. 创建画布布局
fig = plt.figure(figsize=(18, 12))
gs = gridspec.GridSpec(2, 2, height_ratios=[1, 1]) # 2行2列
plt.suptitle('Active Air Suspension & CDC Analysis', fontsize=18, weight='bold')

# ========================================================
# 图 1: 车身高度随速度变化 (Speed vs Height)
# ========================================================
ax1 = fig.add_subplot(gs[0, 0])
# 使用双Y轴，同时展示速度和高度随时间的关系，这样更直观看到"随动"效果
l1 = ax1.plot(time, speed, 'k--', linewidth=2, label='Vehicle Speed')
ax1.set_xlabel('Time (s)', fontsize=12)
ax1.set_ylabel('Speed (km/h)', color='black', fontsize=12)
ax1.grid(True, linestyle='--', alpha=0.5)

ax1_twin = ax1.twinx()
l2 = ax1_twin.plot(time, height_mm, 'b-', linewidth=2.5, label='Body Height')
ax1_twin.set_ylabel('Height (mm)', color='blue', fontsize=12)
ax1_twin.tick_params(axis='y', labelcolor='blue')

# 合并图例
lns = l1 + l2
labs = [l.get_label() for l in lns]
ax1.legend(lns, labs, loc='upper left')
ax1.set_title('(1) Body Height Adaptation with Speed', fontsize=14)

# ========================================================
# 图 2: 充放气流率曲线 (Inflation/Deflation Flow)
# ========================================================
ax2 = fig.add_subplot(gs[0, 1])
ax2.plot(time, flow_rate_g_s, 'm-', linewidth=2)
ax2.set_title('(2) Air Spring Mass Flow Rate (Control Input)', fontsize=14)
ax2.set_xlabel('Time (s)', fontsize=12)
ax2.set_ylabel('Flow Rate (g/s)', fontsize=12)
ax2.grid(True, linestyle='--', alpha=0.5)
ax2.axhline(0, color='black', linewidth=1)

# 标注充放气区域
ax2.fill_between(time, flow_rate_g_s, 0, where=(flow_rate_g_s>0), color='green', alpha=0.1, label='Inflation')
ax2.fill_between(time, flow_rate_g_s, 0, where=(flow_rate_g_s<0), color='red', alpha=0.1, label='Deflation')
ax2.legend(loc='upper right')

# ========================================================
# 图 3: 刚度随时间变化 (Stiffness vs Time)
# ========================================================
ax3 = fig.add_subplot(gs[1, 0])
ax3.plot(time, stiffness_kN_m, 'r-', linewidth=2)
ax3.set_title('(3) Air Spring Stiffness ($k_1$) Variation', fontsize=14)
ax3.set_xlabel('Time (s)', fontsize=12)
ax3.set_ylabel('Stiffness (kN/m)', fontsize=12)
ax3.grid(True, linestyle='--', alpha=0.5)

# 添加说明
ax3.text(time.iloc[int(len(time)*0.8)], stiffness_kN_m.mean(),
         "Stiffness varies with\nAir Pressure & Height",
         bbox=dict(facecolor='white', alpha=0.8))

# ========================================================
# 图 4: 阻尼随时间变化 (CDC Current & Air Damping)
# ========================================================
ax4 = fig.add_subplot(gs[1, 1])
# CDC 电流代表了系统的主动阻尼水平
l3 = ax4.plot(time, cdc_current, 'purple', linewidth=1.5, alpha=0.8, label='CDC Current (Skyhook)')
ax4.set_xlabel('Time (s)', fontsize=12)
ax4.set_ylabel('CDC Current (A)', color='purple', fontsize=12)
ax4.tick_params(axis='y', labelcolor='purple')
ax4.set_ylim(-0.1, 2.0)

# 同时画出空气弹簧内部阻尼 c1
ax4_twin = ax4.twinx()
l4 = ax4_twin.plot(time, air_damping, 'g--', linewidth=1.5, alpha=0.6, label='Air Internal Damping ($c_1$)')
ax4_twin.set_ylabel('Air Damping Coeff (Ns/m)', color='green', fontsize=12)
ax4_twin.tick_params(axis='y', labelcolor='green')

ax4.set_title('(4) Damping Control (CDC & Air Internal)', fontsize=14)
ax4.grid(True, linestyle='--', alpha=0.5)

# 合并图例
lns2 = l3 + l4
labs2 = [l.get_label() for l in lns2]
ax4.legend(lns2, labs2, loc='upper right')

plt.tight_layout()
plt.show()