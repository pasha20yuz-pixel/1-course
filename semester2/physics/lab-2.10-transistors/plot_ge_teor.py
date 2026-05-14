import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

# Экспериментальные данные (германий, прямая ветвь)
U_ge_pr = np.array([0.368, 0.508, 0.562, 0.652, 0.712, 0.766, 0.843, 0.886,
                    0.962, 1.031, 1.071, 1.088, 1.110, 1.138, 1.173])
I_ge_pr_ma = np.array([1.123, 2.49, 3.17, 4.40, 5.28, 6.15, 7.42, 8.16,
                       9.50, 10.77, 11.56, 11.90, 12.31, 13.06, 13.49])

# Параметры теоретической кривой (ток в мА)
I_s_ma = 3.4e-3          # 3,4 мкА = 0,0034 мА
kT_e = 0.026             # тепловой потенциал, В
U_teor = np.linspace(0, 0.4, 200)
I_teor_ma = I_s_ma * np.exp(U_teor / kT_e)   # экспонента в мА

# Построение графика
plt.figure(figsize=(8, 6))
plt.plot(U_teor, I_teor_ma, 'r-', linewidth=1.5, label='Теория (10.8)')
plt.plot(U_ge_pr, I_ge_pr_ma, 'bo', markersize=5, label='Эксперимент (Ge)')

# Настройка осей и сетки (имитация миллиметровки)
ax = plt.gca()
ax.xaxis.set_minor_locator(MultipleLocator(0.01))   # шаг 0,01 В (1 мм)
ax.yaxis.set_minor_locator(MultipleLocator(0.5))    # шаг 0,5 мА (1 мм)
ax.grid(True, which='minor', color='lightgray', linestyle='-', linewidth=0.5)
ax.grid(True, which='major', color='black', linestyle='-', linewidth=1)

# Подписи и пределы
plt.xlabel('U, В')
plt.ylabel('I, мА')
plt.title('Рисунок 3 – Сравнение экспериментальной и теоретической ВАХ\nгерманиевого диода (линейный масштаб)')
plt.legend()
plt.xlim(0, 0.4)
plt.ylim(0, 15)
plt.tight_layout()
plt.savefig('Ge_comparison_mm.png', dpi=300)
plt.show()