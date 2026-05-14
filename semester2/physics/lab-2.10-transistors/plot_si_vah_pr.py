import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator, FixedLocator

# Экспериментальные данные для кремниевого диода (прямая ветвь)
U_si_pr = np.array([0.671, 0.717, 0.736, 0.756, 0.767, 0.779, 0.794, 0.800,
                    0.805, 0.812, 0.818, 0.822, 0.828, 0.833, 0.840])
I_si_pr_ma = np.array([0.98, 2.69, 3.79, 5.49, 6.72, 8.17, 10.39, 11.34,
                       12.19, 13.53, 14.89, 15.54, 17.19, 18.36, 19.8])

# Масштабы: 1 мм по оси X = 0.0025 В, 1 мм по оси Y = 0.25 мА
# Зададим границы осей с небольшим запасом
x_min, x_max = 0.64, 0.86
y_min, y_max = 0, 21

# Размер графика в миллиметрах: по X 80 мм (от 0.65 до 0.85), по Y 80 мм (от 0 до 20)
# Переведём в дюймы для matplotlib (1 дюйм = 25.4 мм), умножим на 2 для чёткости
fig_width = 80 / 25.4 * 2
fig_height = 80 / 25.4 * 2

# Создаём фигуру и оси
fig, ax = plt.subplots(figsize=(fig_width, fig_height), dpi=300)
ax.set_xlim(x_min, x_max)
ax.set_ylim(y_min, y_max)

# Настраиваем деления: мелкие через 1 мм (0.0025 В по X, 0.25 мА по Y)
ax.xaxis.set_minor_locator(MultipleLocator(0.0025))
ax.yaxis.set_minor_locator(MultipleLocator(0.25))

# Крупные деления только в указанных точках
ax.xaxis.set_major_locator(FixedLocator([0.65, 0.70, 0.75, 0.80, 0.85]))
ax.yaxis.set_major_locator(FixedLocator([2.5, 7.5, 12.5, 17.5, 20]))

# Включаем сетку: мелкая — серая, крупная — чёрная
ax.grid(True, which='minor', color='lightgray', linestyle='-', linewidth=0.5)
ax.grid(True, which='major', color='black', linestyle='-', linewidth=1)

# Подписи осей
ax.set_xlabel('U, В')
ax.set_ylabel('I, мА')


# Наносим экспериментальные точки (красные кружки) и соединяем их линией
ax.plot(U_si_pr, I_si_pr_ma, 'ro', markersize=1, label='Экспериментальные точки')
ax.plot(U_si_pr, I_si_pr_ma, 'b-', linewidth=1, alpha=0.7, label='Соединительная линия')

# Легенда
ax.legend()

# Сохраняем в файл
plt.savefig('Si_forward_mm.png', dpi=300)
plt.show()