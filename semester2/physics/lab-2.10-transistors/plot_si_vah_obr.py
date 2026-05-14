import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator, FixedLocator

# Данные для обратной ветви кремниевого диода (модули)
U_si_obr_mv = np.array([-1.110, -1.665, -2.84, -3.85, -4.65, -5.68, -6.60, -7.62,
                         -8.75, -10.03, -11.14, -12.05, -12.79, -13.32, -13.97])
I_si_obr_ma = np.array([-0.22, -0.33, -0.4, -0.55, -0.66, -0.77, -0.88, -0.99,
                         -1.0, -1.1, -1.2, -1.3, -1.4, -1.45, -1.5])

U_pos = -U_si_obr_mv
I_pos = -I_si_obr_ma

# Пределы осей
x_min, x_max = 0, 15   # мВ
y_min, y_max = 0, 1.6  # мА

# Масштаб: 1 мм = 0.2 мВ по X, 1 мм = 0.02 мА по Y
width_mm = (x_max - x_min) / 0.2   # 75 мм
height_mm = (y_max - y_min) / 0.02 # 80 мм

# Размер фигуры в дюймах (с коэффициентом 2 для чёткости)
fig_width = width_mm / 25.4 * 2
fig_height = height_mm / 25.4 * 2

fig, ax = plt.subplots(figsize=(fig_width, fig_height), dpi=300)
ax.set_xlim(x_min, x_max)
ax.set_ylim(y_min, y_max)

# Мелкие деления через 1 мм
ax.xaxis.set_minor_locator(MultipleLocator(0.2))   # 0.2 мВ = 1 мм
ax.yaxis.set_minor_locator(MultipleLocator(0.02))  # 0.02 мА = 1 мм

# Крупные деления с подписями
ax.xaxis.set_major_locator(FixedLocator([0, 5, 10, 15]))
ax.yaxis.set_major_locator(FixedLocator([0, 0.5, 1.0, 1.5]))

# Сетка
ax.grid(True, which='minor', color='lightgray', linestyle='-', linewidth=0.25)
ax.grid(True, which='major', color='black', linestyle='-', linewidth=0.5)

# Подписи осей
ax.set_xlabel('|U|, мВ')
ax.set_ylabel('|I|, мА')

# Экспериментальные точки и соединительная линия
ax.plot(U_pos, I_pos, 'ro', markersize=1, label='Экспериментальные точки')
ax.plot(U_pos, I_pos, 'b-', linewidth=1, alpha=0.7, label='Соединительная линия')

ax.legend()
plt.savefig('Si_reverse_mm.png', dpi=300)
plt.show()