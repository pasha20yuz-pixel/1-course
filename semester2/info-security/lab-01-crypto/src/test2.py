import numpy as np
import matplotlib.pyplot as plt

# Исходные данные
x_m = np.array([10.7, 9.2, 8.2, 6, 5.2, 16, 16.2, 5.2, 5.6, 6.6])
y_m = np.array([14.8, 10.6, 10, 8, 5.6, 12, 12.2, 4.6, 5.2, 6])

# Константы и параметры
R1 = 100  # Ом
R2 = 11000  # Ом
C = 0.27e-6  # Ф
r_mid = 11.00e-3  # м
N1 = 100
N2 = 100
a_x = 5  # В/дел
a_y = 0.2  # В/дел

# Расчетные формулы
l = 2 * np.pi * r_mid  # Средняя длина окружности
h = 1e-3  # Толщина сердечника (предположение)
S = np.pi * ((r_mid**2) - ((r_mid - h)**2))  # Площадь поперечного сечения

# Масштабные коэффициенты
m_H = (N2 * a_x) / (l * R2)
m_B = (R1 * C * a_y) / (S * N1)

# Расчет H и B для каждой точки
H = m_H * x_m
B = m_B * y_m

# Постоянная магнитной проницаемости вакуума
mu_0 = 4 * np.pi * 1e-7  # Гн/м

# Расчет магнитной проницаемости μ
mu = B / (mu_0 * H)

# Создание графиков
plt.figure(figsize=(12, 6))

# График 1: Петля гистерезиса B(H)
plt.subplot(1, 2, 1)
plt.plot(H, B, marker='o', color='blue')
plt.title('Петля гистерезиса B(H)')
plt.xlabel('H, A/m')
plt.ylabel('B, T')
plt.grid(True)

# График 2: Зависимость магнитной проницаемости μ(H)
plt.subplot(1, 2, 2)
plt.plot(H, mu, marker='o', color='green')
plt.title('Зависимость магнитной проницаемости μ(H)')
plt.xlabel('H, A/m')
plt.ylabel('μ')
plt.grid(True)

# Показ графиков
plt.tight_layout()
plt.show()