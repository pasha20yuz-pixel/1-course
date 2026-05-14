import numpy as np
import matplotlib.pyplot as plt

# ==================== ДАННЫЕ (из таблицы) ====================
# Германиевый диод
U_ge_pr = np.array([0.368, 0.508, 0.562, 0.652, 0.712, 0.766, 0.843, 0.886,
                    0.962, 1.031, 1.071, 1.088, 1.110, 1.138, 1.173])
I_ge_pr_ma = np.array([1.123, 2.49, 3.17, 4.40, 5.28, 6.15, 7.42, 8.16,
                       9.50, 10.77, 11.56, 11.90, 12.31, 13.06, 13.49])

U_ge_obr_mv = np.array([-18.4, -42.2, -968, -1700, -2340, -2800, -3440, -4280,
                        -5730, -8840, -7830, -8570, -9390, -11770, -13230])
I_ge_obr_mkA = np.array([-1.3, -2.2, -3.9, -4.3, -4.7, -4.9, -5.2, -5.6,
                         -6.2, -6.6, -7.0, -7.3, -7.6, -8.4, -8.9])

# Кремниевый диод
U_si_pr = np.array([0.671, 0.717, 0.736, 0.756, 0.767, 0.779, 0.794, 0.800,
                    0.805, 0.812, 0.818, 0.822, 0.828, 0.833, 0.840])
I_si_pr_ma = np.array([0.98, 2.69, 3.79, 5.49, 6.72, 8.17, 10.39, 11.34,
                       12.19, 13.53, 14.89, 15.54, 17.19, 18.36, 19.8])

U_si_obr_mv = np.array([-1.110, -1.665, -2.84, -3.85, -4.65, -5.68, -6.60, -7.62,
                        -8.75, -10.03, -11.14, -12.05, -12.79, -13.32, -13.97])
I_si_obr_ma = np.array([-0.22, -0.33, -0.4, -0.55, -0.66, -0.77, -0.88, -0.99,
                        -1.0, -1.1, -1.2, -1.3, -1.4, -1.45, -1.5])

# ==================== РИСУНОК 1: Германий (стиль рис. 10.2) ====================
fig1, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

# ---- Прямая ветвь ----
ax1.plot(U_ge_pr, I_ge_pr_ma, 'bo-', markersize=4, linewidth=1, label='Эксперимент')
ax1.axhline(0, color='black', linewidth=0.8)
ax1.axvline(0, color='black', linewidth=0.8)

# Линейная аппроксимация первых 5 точек
coeffs_pr = np.polyfit(U_ge_pr[:5], I_ge_pr_ma[:5], 1)
U_line = np.linspace(0, 0.5, 10)
I_line = np.polyval(coeffs_pr, U_line)
ax1.plot(U_line, I_line, 'r--', linewidth=1, label='Линейный участок')

# Находим Uотс (пересечение с осью U)
U_ots = -coeffs_pr[1] / coeffs_pr[0]
ax1.plot(U_ots, 0, 'ro', markersize=5)

# Стрелка и подпись Uотс
ax1.annotate('', xy=(U_ots, 0), xytext=(U_ots-0.1, 2),
             arrowprops=dict(arrowstyle='->', color='red', lw=1.5))
ax1.text(U_ots-0.15, 2.5, r'$U_{\text{отс}}$', fontsize=12, color='red')

ax1.set_xlabel('U, В')
ax1.set_ylabel('I, мА')
ax1.set_title('Прямая ветвь (Ge)')
ax1.legend()
ax1.grid(True, linestyle='--', linewidth=0.5)

# ---- Обратная ветвь ----
U_obr_pos = -U_ge_obr_mv   # модуль
I_obr_pos = -I_ge_obr_mkA
ax2.plot(U_obr_pos, I_obr_pos, 'bo-', markersize=4, linewidth=1, label='Эксперимент')
ax2.axhline(0, color='black', linewidth=0.8)
ax2.axvline(0, color='black', linewidth=0.8)

# Линейная аппроксимация первых 5 точек
coeffs_obr = np.polyfit(U_obr_pos[2:8], I_obr_pos[2:8], 1)
U_line_obr = np.linspace(0, U_obr_pos[4], 20)
I_line_obr = np.polyval(coeffs_obr, U_line_obr)
ax2.plot(U_line_obr, I_line_obr, 'g--', linewidth=1, label='Линейный участок')

# Ток насыщения Is (значение при U=0)
I_s_val = coeffs_obr[1]
ax2.plot(0, I_s_val, 'go', markersize=5)

# Стрелка и подпись Is
ax2.annotate('', xy=(0, I_s_val), xytext=(300, I_s_val-1.5),
             arrowprops=dict(arrowstyle='->', color='green', lw=1.5))
ax2.text(320, I_s_val-1.5, r'$I_s$', fontsize=12, color='green')

ax2.set_xlabel('|U|, мВ')
ax2.set_ylabel('|I|, мкА')
ax2.set_title('Обратная ветвь (Ge)')
ax2.legend()
ax2.grid(True, linestyle='--', linewidth=0.5)

fig1.suptitle('Рисунок 1 – Вольт-амперная характеристика германиевого диода', fontsize=14, fontweight='bold')
plt.tight_layout()
plt.savefig('Ge_VAH.png', dpi=300)
plt.show()

# ==================== РИСУНОК 2: Кремний ====================
fig2, (ax3, ax4) = plt.subplots(1, 2, figsize=(12, 5))

# Прямая ветвь (Si)
ax3.plot(U_si_pr, I_si_pr_ma, 'bs-', markersize=4, linewidth=1, label='Эксперимент')
ax3.axhline(0, color='black', linewidth=0.8)
ax3.axvline(0, color='black', linewidth=0.8)
ax3.set_xlabel('U, В')
ax3.set_ylabel('I, мА')
ax3.set_title('Прямая ветвь (Si)')
ax3.legend()
ax3.grid(True, linestyle='--', linewidth=0.5)

# Обратная ветвь (Si)
U_si_obr_pos = -U_si_obr_mv
I_si_obr_pos = -I_si_obr_ma
ax4.plot(U_si_obr_pos, I_si_obr_pos, 'rs-', markersize=4, linewidth=1, label='Эксперимент')
ax4.set_xlabel('|U|, мВ')
ax4.set_ylabel('|I|, мА')
ax4.set_title('Обратная ветвь (Si)')
ax4.legend()
ax4.grid(True, linestyle='--', linewidth=0.5)

fig2.suptitle('Рисунок 2 – Вольт-амперная характеристика кремниевого диода', fontsize=14, fontweight='bold')
plt.tight_layout()
plt.savefig('Si_VAH.png', dpi=300)
plt.show()

# ==================== РИСУНОК 3: Сравнение теории и эксперимента (Ge) ====================
fig3, ax5 = plt.subplots(figsize=(8, 6))

# Эксперимент (ток в амперах)
I_exp_A = I_ge_pr_ma * 1e-3

# Теоретическая кривая
I_s = 15e-6   # ток насыщения в А (из графика)
kT_e = 0.026  # температурный потенциал
U_teor = np.linspace(0, 0.4, 200)
I_teor = I_s * (np.exp(U_teor / kT_e) - 1)

ax5.semilogy(U_teor, I_teor, 'r-', linewidth=1.5, label='Теория (10.8)')
ax5.semilogy(U_ge_pr, I_exp_A, 'bo', markersize=5, label='Эксперимент (Ge)')
ax5.set_xlabel('U, В')
ax5.set_ylabel('I, А')
ax5.set_title('Рисунок 3 – Сравнение экспериментальной и теоретической ВАХ\nгерманиевого диода')
ax5.legend()
ax5.grid(True, which='both', linestyle='--', linewidth=0.5)

plt.tight_layout()
plt.savefig('Ge_comparison.png', dpi=300)
plt.show()