#include "lcg_period.h"
#include <stdint.h>
#include <limits.h>

uint64_t find_period(uint64_t a, uint64_t x0, uint64_t c, uint64_t m) {
    if (m == 0) return 0;
    
    // Защита от переполнения при умножении в формуле
    // Если a == 0 или a == 1, переполнения не будет, но оставляем общую проверку.
    
    uint64_t tortoise = (a * x0 + c) % m;
    uint64_t hare = (a * tortoise + c) % m;
    
    // 1. Поиск точки встречи
    while (tortoise != hare) {
        // Проверка потенциального переполнения (для консервативности)
        if (tortoise >= UINT64_MAX / a || hare >= UINT64_MAX / a) {
            return 0; // Риск переполнения, отказываемся от вычисления
        }
        tortoise = (a * tortoise + c) % m;
        hare = (a * hare + c) % m;
        hare = (a * hare + c) % m;
    }
    
    // 2. Нахождение длины предпериода μ
    uint64_t mu = 0;
    tortoise = x0;
    while (tortoise != hare) {
        tortoise = (a * tortoise + c) % m;
        hare = (a * hare + c) % m;
        mu++;
        if (mu > m) return 0; // Защита от бесконечного цикла
    }
    
    // 3. Нахождение длины периода λ
    uint64_t lam = 1;
    hare = (a * tortoise + c) % m;
    while (tortoise != hare) {
        hare = (a * hare + c) % m;
        lam++;
        if (lam > m) return 0;
    }
    
    return lam;
}