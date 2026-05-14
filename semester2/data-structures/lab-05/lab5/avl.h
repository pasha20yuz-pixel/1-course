#ifndef AVL_TREE_H
#define AVL_TREE_H

// Структура узла АВЛ-дерева
typedef struct Node {
    int key;                // Значение узла
    struct Node* left;      // Указатель на левое поддерево
    struct Node* right;     // Указатель на правое поддерево
    int height;             // Высота узла
    int size;               // Количество узлов в поддереве
} Node;

// Функции для работы с деревом
Node* create_node(int key);
int get_height(Node* node);
int get_size(Node* node);
int balance_factor(Node* node);
void update_node(Node* node);

// Повороты
Node* rotate_right(Node* y);
Node* rotate_left(Node* x);

// Балансировка
Node* balance(Node* node);

// Основные операции
Node* insert_node(Node* root, int key);
Node* delete_node(Node* root, int key);
Node* find_min(Node* node);
Node* find_max(Node* node);
int find_node(Node* root, int key);

// Дополнительные операции
int kth_element(Node* root, int k);
int range_count(Node* root, int l, int r);
int prev_element(Node* root, int x);
int next_element(Node* root, int x);
int node_size(Node* root, int x);

// Обход и очистка
void inorder_print(Node* root);
void free_tree(Node* root);

#endif