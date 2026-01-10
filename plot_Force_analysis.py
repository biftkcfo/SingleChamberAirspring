import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

# 1. 读取数据
csv_file = "active_chassis_data.csv"
try:
    df = pd.read_csv(csv_file, skipinitialspace=True)
except FileNotFoundError:
    print(f"错误：找不到 {csv_file}。请先运行 C++ 程序生成数据。")
    exit()

# 2. 提取并计算数据
time = df["Time"]
f_air = df["AirForce_FL"]      # 空气弹簧力 (提供支撑)
f_cdc = df["CDCForce_FL"]      # CDC阻尼力 (提供控制)
# 计算总悬架输出力 (Total Vertical Force)
f_total = f_air + f_cdc

# 3. 创建画布
fig = plt.figure(figsize=(16, 12))
# 使用 2x2 布局，但上面那张图横跨两列
gs = gridspec.GridSpec(2, 2, height_ratios=[1.5, 1])
plt.suptitle('Suspension Force Analysis (Air + CDC)', fontsize=18, weight='bold')

# ========================================================
# 图 1: 三种力的对比 (总览) - 放在最上面，横跨整行
# ========================================================
ax1 = fig.add_subplot(gs[0, :])
ax1.plot(time, f_total, 'k-', linewidth=2.5, label='Total Force ($F_{total}$)', alpha=0.9)
ax1.plot(time, f_air, 'b--', linewidth=1.5, label='Air Spring Force ($F_{air}$)', alpha=0.7)
# CDC 力通常比较小且高频，单独画可能看不清，这里作为叠加展示
# ax1.plot(time, f_cdc, 'm-', linewidth=1.0, label='CDC Damping Force ($F_{cdc}$)', alpha=0.5)

ax1.set_title('(1) Total Vertical Force Output', fontsize=14)
ax1.set_ylabel('Force (N)', fontsize=12)
ax1.set_xlabel('Time (s)', fontsize=12)
ax1.grid(True, linestyle='--', alpha=0.5)
ax1.legend(loc='upper right', fontsize=12)

# 添加文字标注：解释为什么 Total Force 在波动
avg_force = f_total.mean()
ax1.axhline(y=avg_force, color='g', linestyle=':', linewidth=2, label=f'Static Load (~{avg_force:.0f}N)')
ax1.text(time.iloc[0], avg_force + 500, f" Static Equilibrium Force: {avg_force:.0f} N", color='green', fontweight='bold')

# ========================================================
# 图 2: 单独看 CDC 阻尼力 (高频细节)
# ========================================================
ax2 = fig.add_subplot(gs[1, 0])
ax2.plot(time, f_cdc, 'purple', linewidth=1.5)
ax2.set_title('(2) CDC Damping Force Detail ($F_{cdc}$)', fontsize=14)
ax2.set_ylabel('Damping Force (N)', fontsize=12)
ax2.set_xlabel('Time (s)', fontsize=12)
ax2.grid(True, linestyle='--', alpha=0.5)

# 标注：CDC 力是 0 均值的（只耗能，不支撑）
ax2.axhline(0, color='black', linewidth=1)

# ========================================================
# 图 3: 力与悬架行程的关系 (示功图/刚度特性)
# ========================================================
ax3 = fig.add_subplot(gs[1, 1])
# 横轴是行程 (mm)，纵轴是空气弹簧力
height_mm = df["ActualHeight_FL"] * 1000  # m -> mm
# 注意：ActualHeight 越大表示车身越高（气囊拉长），通常力会变小
# 或者您的定义是 suspension_deflection (压缩为正)
# 这里我们画 Force vs Height 来看刚度非线性
ax3.scatter(height_mm, f_air, c=time, cmap='viridis', s=5, alpha=0.5, label='Air Force vs Height')

ax3.set_title('(3) Air Spring Stiffness Characteristic', fontsize=14)
ax3.set_ylabel('Air Spring Force (N)', fontsize=12)
ax3.set_xlabel('Body Height (mm)', fontsize=12)
ax3.grid(True, linestyle='--', alpha=0.5)

# 添加颜色条说明时间演变
cbar = plt.colorbar(ax3.collections[0], ax=ax3)
cbar.set_label('Time (s)')

plt.tight_layout()
plt.show()