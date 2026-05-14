#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>  // size_t

// Узел цепочки (связный список)
typedef struct HashNode {
    char *key;
    char *value;
    struct HashNode *next;
} HashNode;

// Хэш-таблица
typedef struct HashTable {
    HashNode **buckets;   // массив указателей на списки
    size_t capacity;      // размер массива
    size_t size;          // количество элементов
    double load_factor;   // пороговый коэффициент заполнения
    size_t threshold;     // capacity * load_factor
    size_t collisions;    // счётчик коллизий (для эксперимента)
} HashTable;

// Создание таблицы
HashTable* ht_create(size_t capacity, double load_factor);

// Уничтожение таблицы
void ht_destroy(HashTable *ht);

// Вставка/обновление
void ht_put(HashTable *ht, const char *key, const char *value);

// Поиск
const char* ht_get(HashTable *ht, const char *key);

// Удаление (возвращает значение или NULL)
char* ht_del(HashTable *ht, const char *key);

// Печать таблицы (для отладки)
void ht_print(const HashTable *ht);

// Получить текущий load_factor (size/capacity)
double ht_current_load_factor(const HashTable *ht);

// Получить количество коллизий
size_t ht_collisions(const HashTable *ht);

#endif