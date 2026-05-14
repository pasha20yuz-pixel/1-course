#include "hash_table.h"
#include "hash_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Вспомогательная функция: создание узла
static HashNode* create_node(const char *key, const char *value) {
    HashNode *node = (HashNode*)malloc(sizeof(HashNode));
    if (!node) return NULL;
    node->key = strdup(key);
    node->value = strdup(value);
    node->next = NULL;
    return node;
}

// Освобождение цепочки
static void free_chain(HashNode *head) {
    while (head) {
        HashNode *tmp = head;
        head = head->next;
        free(tmp->key);
        free(tmp->value);
        free(tmp);
    }
}

// Рехэширование при расширении
static void rehash(HashTable *ht) {
    size_t new_capacity = ht->capacity * 2;
    HashNode **new_buckets = (HashNode**)calloc(new_capacity, sizeof(HashNode*));
    if (!new_buckets) return;

    // Пройти по всем старым бакетам и переместить узлы
    for (size_t i = 0; i < ht->capacity; ++i) {
        HashNode *node = ht->buckets[i];
        while (node) {
            HashNode *next = node->next;
            // Вычислить новый индекс
            unsigned int hash = SDBMHash(node->key, strlen(node->key)); // определена в hash_functions.h
            size_t new_idx = hash % new_capacity;
            // Вставка в начало нового бакета
            node->next = new_buckets[new_idx];
            new_buckets[new_idx] = node;
            node = next;
        }
    }
    free(ht->buckets);
    ht->buckets = new_buckets;
    ht->capacity = new_capacity;
    ht->threshold = (size_t)(ht->capacity * ht->load_factor);
}

// Создание таблицы
HashTable* ht_create(size_t capacity, double load_factor) {
    HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) return NULL;
    ht->capacity = capacity;
    ht->size = 0;
    ht->load_factor = load_factor;
    ht->threshold = (size_t)(capacity * load_factor);
    ht->collisions = 0;
    ht->buckets = (HashNode**)calloc(capacity, sizeof(HashNode*));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }
    return ht;
}

void ht_destroy(HashTable *ht) {
    if (!ht) return;
    for (size_t i = 0; i < ht->capacity; ++i)
        free_chain(ht->buckets[i]);
    free(ht->buckets);
    free(ht);
}

void ht_put(HashTable *ht, const char *key, const char *value) {
    if (!ht || !key) return;
    unsigned int hash = SDBMHash(key, strlen(key));
    size_t idx = hash % ht->capacity;

    // Проверка, есть ли уже такой ключ в цепочке
    HashNode *curr = ht->buckets[idx];
    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            // Обновление значения
            free(curr->value);
            curr->value = strdup(value);
            return;
        }
        curr = curr->next;
    }

    // Ключ не найден – вставка нового узла
    // Проверка на необходимость рехэширования
    if (ht->size >= ht->threshold) {
        rehash(ht);
        // После рехэширования пересчитать индекс
        hash = SDBMHash(key, strlen(key));
        idx = hash % ht->capacity;
    }

    // Подсчёт коллизий: если бакет не пуст, то это коллизия
    if (ht->buckets[idx] != NULL) {
        ht->collisions++;
    }

    HashNode *new_node = create_node(key, value);
    // Вставка в начало цепочки
    new_node->next = ht->buckets[idx];
    ht->buckets[idx] = new_node;
    ht->size++;
}

const char* ht_get(HashTable *ht, const char *key) {
    if (!ht || !key) return NULL;
    unsigned int hash = SDBMHash(key, strlen(key));
    size_t idx = hash % ht->capacity;
    HashNode *node = ht->buckets[idx];
    while (node) {
        if (strcmp(node->key, key) == 0)
            return node->value;
        node = node->next;
    }
    return NULL;
}

char* ht_del(HashTable *ht, const char *key) {
    if (!ht || !key) return NULL;
    unsigned int hash = SDBMHash(key, strlen(key));
    size_t idx = hash % ht->capacity;
    HashNode *prev = NULL;
    HashNode *curr = ht->buckets[idx];
    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            if (prev == NULL)
                ht->buckets[idx] = curr->next;
            else
                prev->next = curr->next;
            char *val = curr->value; // нужно вернуть
            free(curr->key);
            free(curr);
            ht->size--;
            return val;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

double ht_current_load_factor(const HashTable *ht) {
    return (double)ht->size / ht->capacity;
}

size_t ht_collisions(const HashTable *ht) {
    return ht->collisions;
}

void ht_print(const HashTable *ht) {
    if (!ht) return;
    for (size_t i = 0; i < ht->capacity; ++i) {
        printf("[%zu] -> ", i);
        HashNode *node = ht->buckets[i];
        while (node) {
            printf("(%s:%s) ", node->key, node->value);
            node = node->next;
        }
        printf("\n");
    }
}