#ifndef LCG_PERIOD_H
#define LCG_PERIOD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Определяет период линейного конгруэнтного генератора.
 * 
 * @param a  множитель (0 ≤ a < m)
 * @param x0 начальное значение (0 ≤ x0 < m)
 * @param c  приращение (0 ≤ c < m)
 * @param m  модуль (m > 0)
 * @return   длина периода, 0 в случае ошибки или если период не найден
 */
uint64_t find_period(uint64_t a, uint64_t x0, uint64_t c, uint64_t m);

#ifdef __cplusplus
}
#endif

#endif // LCG_PERIOD_H