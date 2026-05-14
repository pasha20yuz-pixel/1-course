#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Выбор структуры данных через директиву препроцессора
#ifdef USE_RB_TREE
    #include "rb_tree.h"
    #define TREE_NODE RBNode
    #define insert_node rb_insert
    #define delete_node rb_delete
    #define find_node rb_find
    #define kth_element rb_kth
    #define range_count rb_range_count
    #define prev_element rb_prev
    #define next_element rb_next
    #define node_size rb_node_size
    #define inorder_print rb_inorder_print
    #define free_tree rb_free
#else
    #include "avl.h"
    #define TREE_NODE Node
    #define insert_node insert_node
    #define delete_node delete_node
    #define find_node find_node
    #define kth_element kth_element
    #define range_count range_count
    #define prev_element prev_element
    #define next_element next_element
    #define node_size node_size
    #define inorder_print inorder_print
    #define free_tree free_tree
#endif

int main(int argc, char* argv[]) {
    TREE_NODE* root = NULL;
    char command[20];
    int x, l, r, k;
    
    FILE* input = stdin;
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, "Ошибка: не удалось открыть файл %s\n", argv[1]);
            return 1;
        }
    } else if (argc > 2) {
        fprintf(stderr, "Использование: %s [файл_с_командами]\n", argv[0]);
        return 1;
    }
    
    while (fscanf(input, "%s", command) != EOF) {
        if (strcmp(command, "insert") == 0) {
            fscanf(input, "%d", &x);
            root = insert_node(root, x);
        }
        else if (strcmp(command, "delete") == 0) {
            fscanf(input, "%d", &x);
            root = delete_node(root, x);
        }
        else if (strcmp(command, "find") == 0) {
            fscanf(input, "%d", &x);
            if (find_node(root, x)) printf("YES\n");
            else printf("NO\n");
        }
        else if (strcmp(command, "print") == 0) {
            if (!root) printf("_\n");
            else {
                inorder_print(root);
                printf("\n");
            }
        }
        else if (strcmp(command, "kth") == 0) {
            fscanf(input, "%d", &k);
            int res = kth_element(root, k);
            if (res == -1) printf("NO SUCH ELEMENT\n");
            else printf("%d\n", res);
        }
        else if (strcmp(command, "range_count") == 0) {
            fscanf(input, "%d %d", &l, &r);
            printf("%d\n", range_count(root, l, r));
        }
        else if (strcmp(command, "prev") == 0) {
            fscanf(input, "%d", &x);
            int res = prev_element(root, x);
            if (res == -1) printf("NONE\n");
            else printf("%d\n", res);
        }
        else if (strcmp(command, "next") == 0) {
            fscanf(input, "%d", &x);
            int res = next_element(root, x);
            if (res == -1) printf("NONE\n");
            else printf("%d\n", res);
        }
        else if (strcmp(command, "size") == 0) {
            fscanf(input, "%d", &x);
            int res = node_size(root, x);
            if (res == -1) printf("NO SUCH ELEMENT\n");
            else printf("%d\n", res);
        }
    }
    
    if (input != stdin) fclose(input);
    free_tree(root);
    return 0;
}