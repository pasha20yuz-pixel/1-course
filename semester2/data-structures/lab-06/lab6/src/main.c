#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

#define MAX_LINE 1024

int main(void) {
    HashTable *ht = ht_create(16, 0.75); // Стартовый capacity 16, load_factor 0.75
    if (!ht) {
        fprintf(stderr, "Failed to create hash table\n");
        return 1;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), stdin)) {
        // Удаляем перевод строки
        line[strcspn(line, "\n")] = '\0';
        char *cmd = strtok(line, " ");
        if (!cmd) continue;

        if (strcmp(cmd, "put") == 0) {
            char *key = strtok(NULL, " ");
            char *val = strtok(NULL, " ");
            if (key && val) {
                ht_put(ht, key, val);
                printf("OK\n");
            } else {
                printf("ERROR: put requires key and value\n");
            }
        }
        else if (strcmp(cmd, "get") == 0) {
            char *key = strtok(NULL, " ");
            if (key) {
                const char *val = ht_get(ht, key);
                if (val) printf("%s\n", val);
                else printf("NOT FOUND\n");
            } else {
                printf("ERROR: get requires key\n");
            }
        }
        else if (strcmp(cmd, "del") == 0) {
            char *key = strtok(NULL, " ");
            if (key) {
                char *val = ht_del(ht, key);
                if (val) {
                    printf("%s\n", val);
                    free(val); // освобождаем возвращённую строку
                } else {
                    printf("NOT FOUND\n");
                }
            } else {
                printf("ERROR: del requires key\n");
            }
        }
        else if (strcmp(cmd, "print") == 0) {
            ht_print(ht);
        }
        else if (strcmp(cmd, "exit") == 0) {
            break;
        }
        else {
            printf("Unknown command: %s\n", cmd);
        }
    }

    ht_destroy(ht);
    return 0;
}