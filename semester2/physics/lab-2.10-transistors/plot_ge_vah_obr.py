import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator, FixedLocator

# Данные для обратной ветви германиевого диода (модули)
U_ge_obr_mv = np.array([-18.4, -42.2, -968, -1700, -2340, -2800, -3440, -4280,
                        -5730, -8840, -7830, -8570, -9390, -11770, -13230])
I_ge_obr_mkA = np.array([-1.3, -2.2, -3.9, -4.3, -4.7, -4.9, -5.2, -5.6,
                         -6.2, -6.6, -7.0, -7.3, -7.6, -8.4, -8.9])

U_pos = -U_ge_obr_mv
I_pos = -I_ge_obr_mkA

# Ограничиваем диапазон, чтобы график имел размер 8x8 см при масштабе:
# 1 мм = 50 мВ по X, 1 мм = 0.1 мкА по Y
x_min, x_max = 0, 4000   # мВ → 80 мм = 8 см
y_min, y_max = 0, 8      # мкА → 80 мм = 8 см

# Размер в миллиметрах (он же диапазон, делённый на масштаб)
width_mm = (x_max - x_min) / 50   # 80 мм
height_mm = (y_max - y_min) / 0.1 # 80 мм

# Размер фигуры в дюймах (точно, без коэффициента)
fig_width = width_mm / 25.4   # 80 / 25.4 ≈ 3.15 дюйма
fig_height = height_mm / 25.4

fig, ax = plt.subplots(figsize=(fig_width, fig_height), dpi=300)
ax.set_xlim(x_min, x_max)
ax.set_ylim(y_min, y_max)

# Мелкие деления через 1 мм
ax.xaxis.set_minor_locator(MultipleLocator(50))    # 50 мВ = 1 мм
ax.yaxis.set_minor_locator(MultipleLocator(0.1))   # 0.1 мкА = 1 мм

# Крупные деления (подписанные) – через 1000 мВ по X и через 2 мкА по Y
x_ticks_major = [0, 1000, 2000, 3000, 4000]
y_ticks_major = [0, 2, 4, 6, 8]
ax.xaxis.set_major_locator(FixedLocator(x_ticks_major))
ax.yaxis.set_major_locator(FixedLocator(y_ticks_major))

# Сетка
ax.grid(True, which='minor', color='lightgray', linestyle='-', linewidth=0.5)
ax.grid(True, which='major', color='black', linestyle='-', linewidth=1)

# Подписи осей
ax.set_xlabel('|U|, мВ')
ax.set_ylabel('|I|, мкА')

# Отображаем только те точки, которые попадают в выбранный диапазон
mask = (U_pos <= x_max) & (I_pos <= y_max)
U_plot = U_pos[mask]
I_plot = I_pos[mask]

ax.plot(U_plot, I_plot, 'ro', markersize=1, label='Экспериментальные точки')
ax.plot(U_plot, I_plot, 'b-', linewidth=1, alpha=0.7, label='Соединительная линия')

# Определение Is по первым 5 точкам (все они влезают в диапазон)
coeffs = np.polyfit(U_pos[:5], I_pos[:5], 1)
I_s = coeffs[1]   # ток при U=0
U_line = np.linspace(0, U_pos[4], 20)
I_line = np.polyval(coeffs, U_line)
ax.plot(U_line, I_line, 'g--', linewidth=1, label='Линейная экстраполяция')
ax.plot(0, I_s, 'go', markersize=6)  # точка Is

# Стрелка и подпись Is (позиция подобрана под новый масштаб)
ax.annotate('', xy=(0, I_s), xytext=(500, I_s-0.8),
            arrowprops=dict(arrowstyle='->', color='green', lw=1.5))
ax.text(600, I_s-0.8, r'$I_s$', fontsize=12, color='green')

ax.legend(loc='upper right')
plt.savefig('Ge_reverse_8x8.png', dpi=300)
plt.show()