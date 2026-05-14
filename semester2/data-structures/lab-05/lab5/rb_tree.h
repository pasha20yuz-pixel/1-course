#ifndef RB_TREE_H
#define RB_TREE_H

#include <stdio.h>

// Цвета узлов
typedef enum { RED, BLACK } Color;

// Структура узла красно-чёрного дерева
typedef struct RBNode {
    int key;
    struct RBNode *left, *right, *parent;
    Color color;
    int size;               // количество узлов в поддереве (аугментация)
} RBNode;

// Базовые функции
RBNode* rb_create_node(int key);
int rb_get_size(RBNode *node);
void rb_update_size(RBNode *node);
void rb_update_up(RBNode *node);

// Повороты
void rb_rotate_left(RBNode **root, RBNode *x);
void rb_rotate_right(RBNode **root, RBNode *y);

// Балансировка после вставки и удаления
void rb_insert_fixup(RBNode **root, RBNode *z);
void rb_delete_fixup(RBNode **root, RBNode *x);

// Основные операции (интерфейс, совместимый с АВЛ)
RBNode* rb_insert(RBNode *root, int key);
RBNode* rb_delete(RBNode *root, int key);
RBNode* rb_find_min(RBNode *node);
int rb_find(RBNode *root, int key);
int rb_kth(RBNode *root, int k);
int rb_range_count(RBNode *root, int l, int r);
int rb_prev(RBNode *root, int x);
int rb_next(RBNode *root, int x);
int rb_node_size(RBNode *root, int x);
void rb_inorder_print(RBNode *root);
void rb_free(RBNode *root);

#endif