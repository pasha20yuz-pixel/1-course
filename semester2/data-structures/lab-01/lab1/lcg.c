#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>

// Максимальное значение для 64-битных чисел
#define MAX_VALUE 18446744073709551615ULL

// Структура для хранения аргументов команды
typedef struct {
    char name[20];
    char value[100];
} Argument;

// Функция для вычисления НОД
uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// Проверка числа на простоту (детерминированный тест для небольших чисел)
bool is_prime(uint64_t n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    for (uint64_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// Функция для разложения числа на простые множители
// Возвращает массив делителей и их количество
void prime_factors(uint64_t n, uint64_t factors[], int *count) {
    *count = 0;
    
    // Обработка делителя 2
    while (n % 2 == 0) {
        // Добавляем делитель только если его еще нет
        bool exists = false;
        for (int i = 0; i < *count; i++) {
            if (factors[i] == 2) {
                exists = true;
                break;
            }
        }
        if (!exists && *count < 100) {
            factors[(*count)++] = 2;
        }
        n /= 2;
    }
    
    // Обработка нечетных делителей
    for (uint64_t i = 3; i * i <= n; i += 2) {
        while (n % i == 0) {
            bool exists = false;
            for (int j = 0; j < *count; j++) {
                if (factors[j] == i) {
                    exists = true;
                    break;
                }
            }
            if (!exists && *count < 100) {
                factors[(*count)++] = i;
            }
            n /= i;
        }
    }
    
    // Если остался простой делитель больше квадратного корня
    if (n > 1 && *count < 100) {
        bool exists = false;
        for (int i = 0; i < *count; i++) {
            if (factors[i] == n) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            factors[(*count)++] = n;
        }
    }
}

// Команда get_c: найти все c, взаимно простые с m в диапазоне [cmin, cmax]
void get_c(uint64_t cmin, uint64_t cmax, uint64_t m, FILE *out) {
    if (cmin >= m || cmax >= m || cmin > cmax) {
        fprintf(out, "incorrect parameters\n");
        return;
    }
    
    bool found = false;
    for (uint64_t c = cmin; c <= cmax; c++) {
        if (gcd(c, m) == 1) {
            fprintf(out, "%llu\n", c);
            found = true;
        }
    }
    
    if (!found) {
        fprintf(out, "no solution");
    }
}

// Команда get_a: найти минимальное a, удовлетворяющее условиям теоремы
void get_a(uint64_t m, FILE *out) {
    if (m == 0 || m > MAX_VALUE) {
        fprintf(out, "no solution\n");
        return;
    }
    
    // Находим простые делители m
    uint64_t factors[100];
    int factor_count = 0;
    prime_factors(m, factors, &factor_count);
    
    // Вычисляем НОК всех простых делителей
    uint64_t lcm = 1;
    for (int i = 0; i < factor_count; i++) {
        lcm *= factors[i];
    }
    
    // Если m делится на 4, то lcm должен быть кратен 4
    if (m % 4 == 0) {
        // Убедимся, что lcm кратен 4
        while (lcm % 4 != 0) {
            lcm *= 2;
        }
    }
    
    // Ищем минимальное a: a = k*lcm + 1, где a < m
    for (uint64_t k = 1; k * lcm + 1 < m; k++) {
        uint64_t a = k * lcm + 1;
        
        // Проверяем дополнительные условия
        bool valid = true;
        
        // a-1 должно делиться на все простые делители m
        for (int i = 0; i < factor_count; i++) {
            if ((a - 1) % factors[i] != 0) {
                valid = false;
                break;
            }
        }
        
        // Если m делится на 4, то a-1 должно делиться на 4
        if (m % 4 == 0 && (a - 1) % 4 != 0) {
            valid = false;
        }
        
        if (valid) {
            fprintf(out, "%llu\n", a);
            return;
        }
    }
    
    fprintf(out, "no solution\n");
}

// Команда lcg: сгенерировать последовательность чисел
void lcg_gen(uint64_t a, uint64_t x0, uint64_t c, uint64_t m, uint64_t n, FILE *out) {
    // Проверка корректности параметров
    if (n == 0 || a >= m || x0 >= m || c >= m || m == 0) {
        fprintf(out, "no solution\n");
        return;
    }
    
    uint64_t x = x0;
    for (uint64_t i = 0; i < n; i++) {
        x = (a * x + c) % m;
        fprintf(out, "%llu\n", x);
    }
}

// Функция для определения периода ЛКГ
uint64_t find_period(uint64_t a, uint64_t x0, uint64_t c, uint64_t m) {
    // Используем алгоритм Флойда для поиска цикла
    if (m == 0) return 0;
    
    uint64_t tortoise = (a * x0 + c) % m;
    uint64_t hare = (a * tortoise + c) % m;
    
    // Поиск точки встречи
    while (tortoise != hare) {
        if (tortoise >= UINT64_MAX / a || hare >= UINT64_MAX / a) {
            // Защита от переполнения
            return 0;
        }
        tortoise = (a * tortoise + c) % m;
        hare = (a * hare + c) % m;
        hare = (a * hare + c) % m;
    }
    
    // Находим длину цикла
    uint64_t mu = 0;
    tortoise = x0;
    while (tortoise != hare) {
        tortoise = (a * tortoise + c) % m;
        hare = (a * hare + c) % m;
        mu++;
        if (mu > m) return 0; // Защита от бесконечного цикла
    }
    
    // Находим длину периода
    uint64_t lam = 1;
    hare = (a * tortoise + c) % m;
    while (tortoise != hare) {
        hare = (a * hare + c) % m;
        lam++;
        if (lam > m) return 0;
    }
    
    return lam;
}

// Команда test: тестирование последовательности на случайность
// Функция для вычисления критического значения хи-квадрат (приближённо)
double chi2_critical(int df, double alpha) {
    // Для alpha = 0.05 используем табличные значения для df от 1 до 30
    // Для df > 30 используем приближение: sqrt(2*df)*z + df, где z = 1.645 (верхний 0.05)
    if (df < 1) return 0;
    if (df <= 30) {
        // Таблица критических значений для alpha=0.05 (односторонний верхний хвост)
        // Значения взяты из стандартной таблицы
        static const double table[] = {
            3.841, 5.991, 7.815, 9.488, 11.070, 12.592, 14.067, 15.507, 16.919, 18.307,
            20.483, 21.026, 22.362, 23.685, 24.996, 26.296, 27.587, 28.869, 30.144, 31.410,
            32.671, 33.924, 35.172, 36.415, 37.652, 38.885, 40.113, 41.337, 42.557, 43.773
        };
        if (df <= 30) return table[df-1];
    }
    // Аппроксимация для df > 30
    double z = 1.645; // квантиль нормального распределения для 0.05 (верхний)
    return df + z * sqrt(2.0 * df);
}

void test_chi2(const char *filename, FILE *out) {
    FILE *inp = fopen(filename, "r");
    if (!inp) {
        fprintf(out, "file not found\n");
        return;
    }

    // Сначала прочитаем все числа, чтобы найти min и max и общее количество
    uint64_t *data = NULL;
    size_t capacity = 0;
    size_t n = 0;
    uint64_t min_val = UINT64_MAX;
    uint64_t max_val = 0;
    uint64_t num;

    while (fscanf(inp, "%llu", &num) == 1) {
        if (n >= capacity) {
            capacity = capacity ? capacity * 2 : 1024;
            data = realloc(data, capacity * sizeof(uint64_t));
            if (!data) {
                fprintf(out, "memory error\n");
                fclose(inp);
                return;
            }
        }
        data[n++] = num;
        if (num < min_val) min_val = num;
        if (num > max_val) max_val = num;
    }
    fclose(inp);

    if (n == 0) {
        fprintf(out, "no data\n");
        free(data);
        return;
    }

    if (min_val == max_val) {
        // Все числа одинаковы
        fprintf(out, "All numbers are equal. Not random.\n");
        free(data);
        return;
    }

    // Определяем количество интервалов k
    // Желательно, чтобы ожидаемая частота была не менее 5
    int k = (int)sqrt(n);
    if (k < 1) k = 1;
    double expected = (double)n / k;
    while (expected < 5 && k > 1) {
        k--;
        expected = (double)n / k;
    }
    if (k < 1) k = 1;
    expected = (double)n / k;

    // Подсчет частот
    int *freq = calloc(k, sizeof(int));
    if (!freq) {
        fprintf(out, "memory error\n");
        free(data);
        return;
    }

    double range = (double)(max_val - min_val + 1); // длина диапазона
    for (size_t i = 0; i < n; i++) {
        // индекс интервала
        int idx = (int)((data[i] - min_val) * k / range);
        if (idx >= k) idx = k-1; // на случай погрешности
        freq[idx]++;
    }

    // Вычисление хи-квадрат
    double chi2 = 0.0;
    for (int i = 0; i < k; i++) {
        double diff = freq[i] - expected;
        chi2 += diff * diff / expected;
    }

    int df = k - 1; // степени свободы
    double critical = chi2_critical(df, 0.05);

    // Вывод результатов
    fprintf(out, "Total numbers: %llu\n", (unsigned long long)n);
    fprintf(out, "Range: [%llu, %llu]\n", (unsigned long long)min_val, (unsigned long long)max_val);
    fprintf(out, "Number of intervals: %d\n", k);
    fprintf(out, "Expected frequency per interval: %.2f\n", expected);
    fprintf(out, "Observed frequencies:\n");
    // for (int i = 0; i < k; i++) {
    //     fprintf(out, "  interval %d: %d\n", i+1, freq[i]);
    // }
    fprintf(out, "Chi-square statistic: %.4f\n", chi2);
    fprintf(out, "Degrees of freedom: %d\n", df);
    fprintf(out, "Critical value (0.05): %.4f\n", critical);

    if (chi2 < critical) {
        fprintf(out, "Conclusion: sequence appears uniformly distributed (cannot reject null hypothesis)\n");
    } else {
        fprintf(out, "Conclusion: sequence does not appear uniformly distributed (reject null hypothesis)\n");
    }

    free(freq);
    free(data);
}

// Функция для разбора строки с командой
int parse_command(const char *line, char *command, Argument args[], int *arg_count) {
    char buffer[1000];
    strcpy(buffer, line);
    
    // Разделяем команду и аргументы
    char *token = strtok(buffer, " ");
    if (!token) return 0;
    
    strcpy(command, token);
    *arg_count = 0;
    
    // Обрабатываем аргументы
    while ((token = strtok(NULL, " ")) != NULL) {
        if (*arg_count >= 10) break; // Максимум 10 аргументов
        
        // Разделяем имя и значение аргумента
        char *eq = strchr(token, '=');
        if (!eq) return 0;
        
        *eq = '\0';
        strcpy(args[*arg_count].name, token);
        strcpy(args[*arg_count].value, eq + 1);
        (*arg_count)++;
    }
    
    return 1;
}

// Функция для получения значения аргумента
int get_arg_value(const Argument args[], int arg_count, const char *name, uint64_t *value) {
    for (int i = 0; i < arg_count; i++) {
        if (strcmp(args[i].name, name) == 0) {
            // Проверяем, что значение состоит только из цифр
            for (int j = 0; args[i].value[j]; j++) {
                if (!isdigit(args[i].value[j])) {
                    return 0;
                }
            }
            
            // Преобразуем строку в число
            *value = strtoull(args[i].value, NULL, 10);
            return 1;
        }
    }
    return 0;
}

// Функция для получения строкового значения аргумента
int get_arg_string(const Argument args[], int arg_count, const char *name, char *value) {
    for (int i = 0; i < arg_count; i++) {
        if (strcmp(args[i].name, name) == 0) {
            strcpy(value, args[i].value);
            return 1;
        }
    }
    return 0;
}

// Основная функция
int main() {
    FILE *input = fopen("input.txt", "r");
    FILE *output = fopen("output.txt", "w");
    
    if (!input || !output) {
        printf("Error opening files\n");
        return 1;
    }
    
    // Читаем команду из файла
    char line[1000];
    if (!fgets(line, sizeof(line), input)) {
        fprintf(output, "incorrect command\n");
        fclose(input);
        fclose(output);
        return 0;
    }
    
    // Удаляем символ новой строки
    line[strcspn(line, "\n")] = 0;
    
    // Разбираем команду
    char command[20];
    Argument args[10];
    int arg_count;
    
    if (!parse_command(line, command, args, &arg_count)) {
        fprintf(output, "incorrect command\n");
        fclose(input);
        fclose(output);
        return 0;
    }
    
    // Обрабатываем команды
    if (strcmp(command, "get_c") == 0) {
        // Проверяем наличие обязательных аргументов
        uint64_t cmin, cmax, m;
        if (arg_count >= 3 && 
            get_arg_value(args, arg_count, "cmin", &cmin) &&
            get_arg_value(args, arg_count, "cmax", &cmax) &&
            get_arg_value(args, arg_count, "m", &m)) {
            get_c(cmin, cmax, m, output);
        } else {
            fprintf(output, "incorrect command\n");
        }
    }
    else if (strcmp(command, "get_a") == 0) {
        // Проверяем наличие обязательных аргументов
        uint64_t m;
        if (arg_count >= 1 && get_arg_value(args, arg_count, "m", &m)) {
            get_a(m, output);
        } else {
            fprintf(output, "incorrect command\n");
        }
    }
    else if (strcmp(command, "lcg") == 0) {
        // Проверяем наличие обязательных аргументов
        uint64_t a, x0, c, m, n;
        if (arg_count >= 5 &&
            get_arg_value(args, arg_count, "a", &a) &&
            get_arg_value(args, arg_count, "x0", &x0) &&
            get_arg_value(args, arg_count, "c", &c) &&
            get_arg_value(args, arg_count, "m", &m) &&
            get_arg_value(args, arg_count, "n", &n)) {
            lcg_gen(a, x0, c, m, n, output);
        } else {
            fprintf(output, "incorrect command\n");
        }
    }
    else if (strcmp(command, "test") == 0) {
        // Проверяем наличие обязательных аргументов
        char filename[100];
        if (arg_count >= 1 && get_arg_string(args, arg_count, "inp", filename)) {
            test_chi2(filename, output);
        } else {
            fprintf(output, "incorrect command\n");
        }
    }
    else {
        fprintf(output, "incorrect command\n");
    }
    
    fclose(input);
    fclose(output);
    
    return 0;
}