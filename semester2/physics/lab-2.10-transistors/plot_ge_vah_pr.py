import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator, FixedLocator

# Данные для прямой ветви германиевого диода
U_ge_pr = np.array([0.368, 0.508, 0.562, 0.652, 0.712, 0.766, 0.843, 0.886,
                    0.962, 1.031, 1.071, 1.088, 1.110, 1.138, 1.173])
I_ge_pr_ma = np.array([1.123, 2.49, 3.17, 4.40, 5.28, 6.15, 7.42, 8.16,
                       9.50, 10.77, 11.56, 11.90, 12.31, 13.06, 13.49])

# Масштабы: 1 мм = 0.01 В по X, 1 мм = 0.25 мА по Y
x_min, x_max = 0.30, 1.25   # В
y_min, y_max = 0, 15        # мА

# Размеры в миллиметрах
width_mm = (x_max - x_min) / 0.01   # 95 мм
height_mm = (y_max - y_min) / 0.25  # 60 мм

# Размер фигуры в дюймах (с запасом для чёткости)
fig_width = width_mm / 25.4 * 2
fig_height = height_mm / 25.4 * 2

fig, ax = plt.subplots(figsize=(fig_width, fig_height), dpi=300)
ax.set_xlim(x_min, x_max)
ax.set_ylim(y_min, y_max)

# Мелкие деления через 1 мм
ax.xaxis.set_minor_locator(MultipleLocator(0.01))   # 0.01 В = 1 мм
ax.yaxis.set_minor_locator(MultipleLocator(0.25))   # 0.25 мА = 1 мм

# Крупные деления (подписанные)
ax.xaxis.set_major_locator(FixedLocator([0.4, 0.6, 0.8, 1.0, 1.2]))
ax.yaxis.set_major_locator(FixedLocator([2.5, 5.0, 7.5, 10.0, 12.5]))

# Сетка
ax.grid(True, which='minor', color='lightgray', linestyle='-', linewidth=0.5)
ax.grid(True, which='major', color='black', linestyle='-', linewidth=1)

# Подписи осей
ax.set_xlabel('U, В')
ax.set_ylabel('I, мА')

# Экспериментальные точки и соединительная линия
ax.plot(U_ge_pr, I_ge_pr_ma, 'ro', markersize=1, label='Экспериментальные точки')
ax.plot(U_ge_pr, I_ge_pr_ma, 'b-', linewidth=1, alpha=0.7, label='Соединительная линия')

# Определение Uотс по первым 5 точкам
coeffs = np.polyfit(U_ge_pr[:5], I_ge_pr_ma[:5], 1)
U_ots = -coeffs[1] / coeffs[0]   # пересечение с осью U (I=0)
# Экстраполяция линейного участка
U_line = np.linspace(0.3, 0.7, 10)
I_line = np.polyval(coeffs, U_line)
ax.plot(U_line, I_line, 'g--', linewidth=1, label='Линейная экстраполяция')
# Точка пересечения
ax.plot(U_ots, 0, 'go', markersize=6)
# Стрелка и подпись Uотс
ax.annotate('', xy=(U_ots, 0), xytext=(U_ots-0.1, 2),
            arrowprops=dict(arrowstyle='->', color='green', lw=1.5))
ax.text(U_ots-0.15, 2.5, r'$U_{\text{отс}}$', fontsize=12, color='green')

ax.legend(loc='upper left')
plt.savefig('Ge_forward_mm.png', dpi=300)
plt.show()