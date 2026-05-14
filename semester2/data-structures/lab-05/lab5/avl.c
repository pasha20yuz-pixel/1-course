#include <stdio.h>
#include <stdlib.h>
#include "avl.h"

/**
 * Создание нового узла
 * @param key - значение узла
 * @return указатель на новый узел
 */
Node* create_node(int key) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;  // Новый узел имеет высоту 1
    node->size = 1;    // В поддереве 1 узел
    return node;
}

/**
 * Получение высоты узла
 * @param node - указатель на узел
 * @return высота (0 для NULL)
 */
int get_height(Node* node) {
    return node ? node->height : 0;
}

/**
 * Получение размера (количества узлов) поддерева
 * @param node - указатель на узел
 * @return размер (0 для NULL)
 */
int get_size(Node* node) {
    return node ? node->size : 0;
}

/**
 * Вычисление баланс-фактора узла
 * @param node - указатель на узел
 * @return разница высот левого и правого поддеревьев
 */
int balance_factor(Node* node) {
    return node ? get_height(node->left) - get_height(node->right) : 0;
}

/**
 * Обновление высоты и размера узла
 * @param node - указатель на узел
 */
void update_node(Node* node) {
    if (node) {
        // Высота = максимальная высота поддеревьев + 1
        int left_h = get_height(node->left);
        int right_h = get_height(node->right);
        node->height = (left_h > right_h ? left_h : right_h) + 1;
        
        // Размер = размер левого + размер правого + 1
        node->size = get_size(node->left) + get_size(node->right) + 1;
    }
}

/**
 * Правый поворот
 * Используется при лево-левом дисбалансе
 * 
 *        y                    x
 *       / \                  / \
 *      x   T3     =>        T1  y
 *     / \                      / \
 *    T1 T2                    T2 T3
 * 
 * @param y - корень поддерева с дисбалансом
 * @return новый корень поддерева
 */
Node* rotate_right(Node* y) {
    Node* x = y->left;      // x становится новым корнем
    Node* T2 = x->right;    // Сохраняем правое поддерево x
    
    // Выполняем поворот
    x->right = y;
    y->left = T2;
    
    // Обновляем высоты и размеры
    update_node(y);
    update_node(x);
    
    return x;
}

/**
 * Левый поворот
 * Используется при право-правом дисбалансе
 * 
 *      x                          y
 *     / \                        / \
 *    T1  y         =>           x   T3
 *       / \                    / \
 *      T2 T3                  T1 T2
 * 
 * @param x - корень поддерева с дисбалансом
 * @return новый корень поддерева
 */
Node* rotate_left(Node* x) {
    Node* y = x->right;     // y становится новым корнем
    Node* T2 = y->left;     // Сохраняем левое поддерево y
    
    // Выполняем поворот
    y->left = x;
    x->right = T2;
    
    // Обновляем высоты и размеры
    update_node(x);
    update_node(y);
    
    return y;
}

/**
 * Балансировка узла
 * Проверяет баланс-фактор и выполняет необходимые повороты
 * @param node - узел для балансировки
 * @return новый корень поддерева после балансировки
 */
Node* balance(Node* node) {
    if (!node) return NULL;
    
    // Обновляем высоту и размер текущего узла
    update_node(node);
    
    int bf = balance_factor(node);
    
    // Лево-левый случай (правый поворот)
    if (bf > 1 && balance_factor(node->left) >= 0) {
        return rotate_right(node);
    }
    
    // Лево-правый случай (левый поворот левого поддерева, затем правый поворот)
    if (bf > 1 && balance_factor(node->left) < 0) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    
    // Право-правый случай (левый поворот)
    if (bf < -1 && balance_factor(node->right) <= 0) {
        return rotate_left(node);
    }
    
    // Право-левый случай (правый поворот правого поддерева, затем левый поворот)
    if (bf < -1 && balance_factor(node->right) > 0) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    
    return node;
}

/**
 * Вставка узла в дерево
 * @param root - корень дерева
 * @param key - вставляемое значение
 * @return новый корень дерева
 */
Node* insert_node(Node* root, int key) {
    // 1. Обычная вставка BST
    if (!root) {
        return create_node(key);
    }
    
    if (key < root->key) {
        root->left = insert_node(root->left, key);
    } else if (key > root->key) {
        root->right = insert_node(root->right, key);
    } else {
        // Ключ уже существует - ничего не делаем
        return root;
    }
    
    // 2. Балансировка
    return balance(root);
}

/**
 * Поиск минимального узла в поддереве
 * @param node - корень поддерева
 * @return узел с минимальным значением
 */
Node* find_min(Node* node) {
    Node* current = node;
    while (current && current->left) {
        current = current->left;
    }
    return current;
}

/**
 * Поиск максимального узла в поддереве
 * @param node - корень поддерева
 * @return узел с максимальным значением
 */
Node* find_max(Node* node) {
    Node* current = node;
    while (current && current->right) {
        current = current->right;
    }
    return current;
}

/**
 * Удаление узла из дерева
 * @param root - корень дерева
 * @param key - удаляемое значение
 * @return новый корень дерева
 */
Node* delete_node(Node* root, int key) {
    if (!root) return NULL;
    
    // 1. Поиск и удаление узла
    if (key < root->key) {
        root->left = delete_node(root->left, key);
    } else if (key > root->key) {
        root->right = delete_node(root->right, key);
    } else {
        // Нашли узел для удаления
        if (!root->left || !root->right) {
            // Узел имеет 0 или 1 ребенка
            Node* temp = root->left ? root->left : root->right;
            free(root);
            return temp;
        } else {
            // Узел имеет двух детей
            // Находим inorder-преемника (минимальный в правом поддереве)
            Node* temp = find_min(root->right);
            // Копируем его значение в текущий узел
            root->key = temp->key;
            // Удаляем inorder-преемника
            root->right = delete_node(root->right, temp->key);
        }
    }
    
    // 2. Балансировка
    return balance(root);
}

/**
 * Поиск узла в дереве
 * @param root - корень дерева
 * @param key - искомое значение
 * @return 1 если найден, 0 если нет
 */
int find_node(Node* root, int key) {
    if (!root) return 0;
    
    if (key == root->key) return 1;
    if (key < root->key) return find_node(root->left, key);
    return find_node(root->right, key);
}

/**
 * Поиск k-го по порядку элемента (0-индексация)
 * @param root - корень дерева
 * @param k - индекс искомого элемента
 * @return значение элемента или -1 если не найден
 */
int kth_element(Node* root, int k) {
    if (!root || k < 0 || k >= get_size(root)) return -1;
    
    int left_size = get_size(root->left);
    
    if (k == left_size) {
        return root->key;
    } else if (k < left_size) {
        return kth_element(root->left, k);
    } else {
        return kth_element(root->right, k - left_size - 1);
    }
}

/**
 * Подсчет количества элементов в диапазоне [l, r]
 * @param root - корень дерева
 * @param l - левая граница
 * @param r - правая граница
 * @return количество элементов в диапазоне
 */
int range_count(Node* root, int l, int r) {
    if (!root) return 0;
    
    int count = 0;
    
    // Если текущее значение в диапазоне
    if (root->key >= l && root->key <= r) {
        count = 1;
    }
    
    // Если левая граница меньше текущего, идем влево
    if (l < root->key) {
        count += range_count(root->left, l, r);
    }
    
    // Если правая граница больше текущего, идем вправо
    if (r > root->key) {
        count += range_count(root->right, l, r);
    }
    
    return count;
}

/**
 * Поиск предшественника (максимального элемента, меньшего x)
 * @param root - корень дерева
 * @param x - опорное значение
 * @return значение предшественника или -1, если нет
 */
int prev_element(Node* root, int x) {
    Node* current = root;
    int result = -1;  // -1 означает отсутствие
    
    while (current) {
        if (current->key < x) {
            result = current->key;  // Текущий - потенциальный предшественник
            current = current->right; // Ищем больший, но всё ещё меньший x
        } else {
            current = current->left; // Идем влево, чтобы найти меньший
        }
    }
    
    return result;
}

/**
 * Поиск последователя (минимального элемента, большего x)
 * @param root - корень дерева
 * @param x - опорное значение
 * @return значение последователя или -1, если нет
 */
int next_element(Node* root, int x) {
    Node* current = root;
    int result = -1;  // -1 означает отсутствие
    
    while (current) {
        if (current->key > x) {
            result = current->key;  // Текущий - потенциальный последователь
            current = current->left; // Ищем меньший, но всё ещё больший x
        } else {
            current = current->right; // Идем вправо, чтобы найти больший
        }
    }
    
    return result;
}

/**
 * Получение размера поддерева для узла со значением x
 * @param root - корень дерева
 * @param x - значение узла
 * @return размер поддерева или -1, если узел не найден
 */
int node_size(Node* root, int x) {
    if (!root) return -1;
    
    if (x == root->key) {
        return root->size;
    } else if (x < root->key) {
        return node_size(root->left, x);
    } else {
        return node_size(root->right, x);
    }
}

/**
 * Вывод дерева в порядке возрастания (in-order обход)
 * @param root - корень дерева
 */
void inorder_print(Node* root) {
    if (root) {
        inorder_print(root->left);
        printf("%d ", root->key);
        inorder_print(root->right);
    }
}

/**
 * Освобождение памяти дерева
 * @param root - корень дерева
 */
void free_tree(Node* root) {
    if (root) {
        free_tree(root->left);
        free_tree(root->right);
        free(root);
    }
}