#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef USE_RB_TREE
    #include "rb_tree.h"
    #define TREE_NODE RBNode
    #define insert_node rb_insert
    #define delete_node rb_delete
    #define find_node rb_find
    #define kth_element rb_kth
    #define range_count rb_range_count
    #define free_tree rb_free
#else
    #include "avl.h"
    #define TREE_NODE Node
    #define insert_node insert_node
    #define delete_node delete_node
    #define find_node find_node
    #define kth_element kth_element
    #define range_count range_count
    #define free_tree free_tree
#endif

// Генерация массива уникальных случайных чисел
int* generate_unique_numbers(int n) {
    int *arr = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i + 1;
    // Перемешивание Фишера-Йетса
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
    return arr;
}

// Измерение времени вставки N элементов
double measure_insert(TREE_NODE **root, int *values, int n) {
    clock_t start = clock();
    for (int i = 0; i < n; i++) {
        *root = insert_node(*root, values[i]);
    }
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Измерение времени поиска (queries раз)
double measure_find(TREE_NODE *root, int *values, int n, int queries) {
    clock_t start = clock();
    for (int i = 0; i < queries; i++) {
        int idx = rand() % n;
        find_node(root, values[idx]);
    }
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Измерение времени удаления первых n/2 элементов (уже случайно перемешанных)
double measure_delete(TREE_NODE **root, int *values, int n) {
    clock_t start = clock();
    for (int i = 0; i < n / 2; i++) {
        *root = delete_node(*root, values[i]);
    }
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Измерение времени kth и range_count (queries раз)
double measure_kth_range(TREE_NODE *root, int n, int queries) {
    clock_t start = clock();
    for (int i = 0; i < queries; i++) {
        int k = rand() % n;
        kth_element(root, k);
        int l = rand() % (n + 1);
        int r = l + rand() % (n - l + 1);
        range_count(root, l, r);
    }
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

int main(int argc, char *argv[]) {
    // Определяем выходной поток: если передан аргумент, открываем файл
    FILE *out = stdout;
    if (argc == 2) {
        out = fopen(argv[1], "w");
        if (!out) {
            fprintf(stderr, "Ошибка: не могу создать файл %s\n", argv[1]);
            return 1;
        }
    } else if (argc > 2) {
        fprintf(stderr, "Использование: %s [выходной_файл.csv]\n", argv[0]);
        return 1;
    }

    srand(time(NULL));
    int sizes[] = {1000, 10000, 100000};
    int repeats = 3;   // количество повторений для усреднения

    fprintf(out, "N,InsertTime,FindTime,DeleteTime,KthRangeTime\n");

    for (int s = 0; s < 3; s++) {
        int N = sizes[s];
        double insert_total = 0, find_total = 0, delete_total = 0, kr_total = 0;

        for (int r = 0; r < repeats; r++) {
            int *values = generate_unique_numbers(N);
            TREE_NODE *root = NULL;

            insert_total += measure_insert(&root, values, N);
            find_total   += measure_find(root, values, N, 1000);
            delete_total += measure_delete(&root, values, N);
            kr_total     += measure_kth_range(root, N / 2, 1000);

            free_tree(root);
            free(values);
        }

        fprintf(out, "%d,%.5f,%.5f,%.5f,%.5f\n",
                N,
                insert_total / repeats,
                find_total   / repeats,
                delete_total / repeats,
                kr_total     / repeats);
    }

    if (out != stdout) fclose(out);
    return 0;
}