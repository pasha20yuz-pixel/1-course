#include <stdio.h>
#include <stdlib.h>
#include "rb_tree.h"

RBNode* rb_create_node(int key) {
    RBNode *node = (RBNode*)malloc(sizeof(RBNode));
    node->key = key;
    node->left = node->right = node->parent = NULL;
    node->color = RED;
    node->size = 1;
    return node;
}

int rb_get_size(RBNode *node) {
    return node ? node->size : 0;
}

void rb_update_size(RBNode *node) {
    if (node) {
        node->size = rb_get_size(node->left) + rb_get_size(node->right) + 1;
    }
}

void rb_update_up(RBNode *node) {
    while (node) {
        rb_update_size(node);
        node = node->parent;
    }
}

void rb_rotate_left(RBNode **root, RBNode *x) {
    RBNode *y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) *root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    rb_update_size(x);
    rb_update_size(y);
}

void rb_rotate_right(RBNode **root, RBNode *y) {
    RBNode *x = y->left;
    y->left = x->right;
    if (x->right) x->right->parent = y;
    x->parent = y->parent;
    if (!y->parent) *root = x;
    else if (y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;
    rb_update_size(y);
    rb_update_size(x);
}

void rb_insert_fixup(RBNode **root, RBNode *z) {
    while (z != *root && z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode *y = z->parent->parent->right;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rb_rotate_left(root, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rb_rotate_right(root, z->parent->parent);
            }
        } else {
            RBNode *y = z->parent->parent->left;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rb_rotate_right(root, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rb_rotate_left(root, z->parent->parent);
            }
        }
    }
    (*root)->color = BLACK;
}

RBNode* rb_insert(RBNode *root, int key) {
    RBNode *z = rb_create_node(key);
    RBNode *y = NULL;
    RBNode *x = root;
    while (x) {
        y = x;
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else { free(z); return root; }
    }
    z->parent = y;
    if (!y) root = z;
    else if (key < y->key) y->left = z;
    else y->right = z;
    rb_update_up(z);
    rb_insert_fixup(&root, z);
    return root;
}

RBNode* rb_find_min(RBNode *node) {
    while (node && node->left) node = node->left;
    return node;
}

void rb_transplant(RBNode **root, RBNode *u, RBNode *v) {
    if (!u->parent) *root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

void rb_delete_fixup(RBNode **root, RBNode *x) {
    while (x != *root && (x == NULL || x->color == BLACK)) {
        if (x == NULL) break;  // страховка
        if (x == x->parent->left) {
            RBNode *w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rb_rotate_left(root, x->parent);
                w = x->parent->right;
            }
            if ((!w->left || w->left->color == BLACK) &&
                (!w->right || w->right->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (!w->right || w->right->color == BLACK) {
                    if (w->left) w->left->color = BLACK;
                    w->color = RED;
                    rb_rotate_right(root, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w->right) w->right->color = BLACK;
                rb_rotate_left(root, x->parent);
                x = *root;
            }
        } else {
            RBNode *w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rb_rotate_right(root, x->parent);
                w = x->parent->left;
            }
            if ((!w->right || w->right->color == BLACK) &&
                (!w->left || w->left->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (!w->left || w->left->color == BLACK) {
                    if (w->right) w->right->color = BLACK;
                    w->color = RED;
                    rb_rotate_left(root, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w->left) w->left->color = BLACK;
                rb_rotate_right(root, x->parent);
                x = *root;
            }
        }
    }
    if (x) x->color = BLACK;
}

RBNode* rb_delete(RBNode *root, int key) {
    RBNode *z = root;
    while (z) {
        if (key < z->key) z = z->left;
        else if (key > z->key) z = z->right;
        else break;
    }
    if (!z) return root;

    RBNode *y = z;
    RBNode *x = NULL;
    Color y_original_color = y->color;

    if (!z->left) {
        x = z->right;
        rb_transplant(&root, z, z->right);
    } else if (!z->right) {
        x = z->left;
        rb_transplant(&root, z, z->left);
    } else {
        y = rb_find_min(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            if (x) x->parent = y;
        } else {
            rb_transplant(&root, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        rb_transplant(&root, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
        // обновляем размеры: от y (новый корень поддерева) и от x
        rb_update_up(y);
    }
    // обновляем размеры от x вверх
    if (x) rb_update_up(x);
    else if (y->parent) rb_update_up(y->parent);
    else if (root) rb_update_up(root);

    if (y_original_color == BLACK) {
        rb_delete_fixup(&root, x ? x : (y->parent ? y->parent : NULL));
    }
    free(z);
    return root;
}

int rb_find(RBNode *root, int key) {
    while (root) {
        if (key == root->key) return 1;
        if (key < root->key) root = root->left;
        else root = root->right;
    }
    return 0;
}

int rb_kth(RBNode *root, int k) {
    if (!root || k < 0 || k >= rb_get_size(root)) return -1;
    int left_size = rb_get_size(root->left);
    if (k == left_size) return root->key;
    if (k < left_size) return rb_kth(root->left, k);
    return rb_kth(root->right, k - left_size - 1);
}

int rb_range_count(RBNode *root, int l, int r) {
    if (!root) return 0;
    int count = (root->key >= l && root->key <= r) ? 1 : 0;
    if (l < root->key) count += rb_range_count(root->left, l, r);
    if (r > root->key) count += rb_range_count(root->right, l, r);
    return count;
}

int rb_prev(RBNode *root, int x) {
    int res = -1;
    while (root) {
        if (root->key < x) {
            res = root->key;
            root = root->right;
        } else {
            root = root->left;
        }
    }
    return res;
}

int rb_next(RBNode *root, int x) {
    int res = -1;
    while (root) {
        if (root->key > x) {
            res = root->key;
            root = root->left;
        } else {
            root = root->right;
        }
    }
    return res;
}

int rb_node_size(RBNode *root, int x) {
    while (root) {
        if (x == root->key) return root->size;
        if (x < root->key) root = root->left;
        else root = root->right;
    }
    return -1;
}

void rb_inorder_print(RBNode *root) {
    if (root) {
        rb_inorder_print(root->left);
        printf("%d ", root->key);
        rb_inorder_print(root->right);
    }
}

void rb_free(RBNode *root) {
    if (root) {
        rb_free(root->left);
        rb_free(root->right);
        free(root);
    }
}