#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


// Структура для хранения статистики сортировки
typedef struct {
    long long comparisons;  // количество сравнений
    long long movements;    // количество перемещений
    double time_taken;      // время выполнения
} SortStats;


// Вспомогательные функции
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int* copy_array(const int* source, int size) {
    int* dest = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        dest[i] = source[i];
    }
    return dest;
}

void print_array_full(const int* arr, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i != size - 1) {
            printf(", ");
        }
    }
}

// Функция проверки отсортированности
int check_sorted(const int* arr, int size) {
    for (int i = 0; i < size - 1; i++) {
        if (arr[i] > arr[i + 1]) return 0;
    }
    return 1;
}


// АЛГОРИТМЫ СОРТИРОВКИ

// QUICKSORT 
void quick_sort_recursive(int* arr, int low, int high, SortStats* stats) {
    if (low >= high) return;

    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        stats->comparisons++;
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
            stats->movements += 3;  // 3 присваивания в swap
        }
    }

    swap(&arr[i + 1], &arr[high]);
    stats->movements += 3;
    int pivot_pos = i + 1;

    quick_sort_recursive(arr, low, pivot_pos - 1, stats);
    quick_sort_recursive(arr, pivot_pos + 1, high, stats);
}

void quick_sort(int* arr, int size, SortStats* stats) {
    stats->comparisons = 0;
    stats->movements = 0;
    quick_sort_recursive(arr, 0, size - 1, stats);
}

// СОРТИРОВКА ШЕЛЛА 
void shell_sort(int* arr, int size, SortStats* stats) {
    stats->comparisons = 0;
    stats->movements = 0;
    
    for (int gap = size / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < size; i++) {
            int temp = arr[i];
            stats->movements++;
            int j;
            for (j = i; j >= gap; j -= gap) {
                stats->comparisons++;
                if (arr[j - gap] > temp) {
                    arr[j] = arr[j - gap];
                    stats->movements++;
                }
                else break;
            }
            arr[j] = temp;
            stats->movements++;
        }
    }
}

// СОРТИРОВКА ВЫБОРОМ 
void selection_sort(int* arr, int size, SortStats* stats) {
    stats->comparisons = 0;
    stats->movements = 0;
    
    for (int i = 0; i < size - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < size; j++) {
            stats->comparisons++;
            if (arr[j] < arr[min_idx]) min_idx = j;
        }
        if (min_idx != i) {
            swap(&arr[i], &arr[min_idx]);
            stats->movements += 3;
        }
    }
}

// ШЕЙКЕРНАЯ СОРТИРОВКА 
void shaker_sort(int* arr, int size, SortStats* stats) {
    stats->comparisons = 0;
    stats->movements = 0;
    
    int left = 0, right = size - 1;
    int swapped = 1;

    while (left < right && swapped) {
        swapped = 0;
        for (int i = left; i < right; i++) {
            stats->comparisons++;
            if (arr[i] > arr[i + 1]) {
                swap(&arr[i], &arr[i + 1]);
                stats->movements += 3;
                swapped = 1;
                
            }
        }
        right--;
        for (int i = right; i > left; i--) {
            stats->comparisons++;
            if (arr[i] < arr[i - 1]) {
                swap(&arr[i], &arr[i - 1]);
                stats->movements += 3;
                swapped = 1;
            }
        }
        left++;
    }
}


// ФУНКЦИИ ДЛЯ РАБОТЫ С ВХОДНЫМИ ДАННЫМИ

int* read_array_from_file(const char* filename, int* size) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }
    
    int count = 0;
    int temp;
    while (fscanf(file, "%d", &temp) == 1) {
        count++;
    }
    
    rewind(file);
    
    int* arr = (int*)malloc(count * sizeof(int));
    for (int i = 0; i < count; i++) {
        fscanf(file, "%d", &arr[i]);
    }
    
    fclose(file);
    *size = count;
    return arr;
}

int* generate_random_array(int size) {
    int* arr = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 100000;
    }
    return arr;
}


// ОСНОВНАЯ ПРОГРАММА
int main() {
    srand(time(NULL));
    
    int* original_array = NULL;
    int size = 0;
    char filename[100];
    
    // Чтение имени файла из stdin
    if (fgets(filename, sizeof(filename), stdin) != NULL) {
        filename[strcspn(filename, "\n")] = 0;
    }
    
    // Инициализация массива
    if (strlen(filename) > 0) {
        original_array = read_array_from_file(filename, &size);
        if (!original_array) {
            size = 15000;
            original_array = generate_random_array(size);
        }
    } else {
        size = 15000;
        original_array = generate_random_array(size);
    }
    
    // Подготовка к сортировкам
    void (*sort_functions[])(int*, int, SortStats*) = {
        selection_sort,
        shaker_sort,
        quick_sort,
        shell_sort
    };
    
    const char* sort_names[] = {
        "Selection",
        "Shaker",
        "Quick",
        "Shell"
    };
    
    int* sorted_arrays[4];
    SortStats stats_array[4];
    
    // Выполнение сортировок
    for (int i = 0; i < 4; i++) {
        sorted_arrays[i] = copy_array(original_array, size);
        
        clock_t start_time = clock();
        sort_functions[i](sorted_arrays[i], size, &stats_array[i]);
        clock_t end_time = clock();
        
        stats_array[i].time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    }
    
    // Вывод отсортированных массивов (ПЕРВЫМИ!)
    for (int i = 0; i < 4; i++) {
        printf("[+] SORT \"%s\": ", sort_names[i]);
        print_array_full(sorted_arrays[i], size);
        printf("\n");
    }
    
    // Вывод статистики (ПОСЛЕ массивов!)
    for (int i = 0; i < 4; i++) {
        printf("%s: time=%.3fs, comparisons=%lld, moves=%lld, correct=%s\n",
               sort_names[i],
               stats_array[i].time_taken,
               stats_array[i].comparisons,
               stats_array[i].movements,
               check_sorted(sorted_arrays[i], size) ? "yes" : "no");
    }
    
    // Очистка памяти
    for (int i = 0; i < 4; i++) {
        free(sorted_arrays[i]);
    }
    free(original_array);
    
    return 0;
}