// main.c – исправленный полный текст
#include <stdio.h>
#include <string.h>
#include "include/types.h"
#include "include/cmd.h"
#include "include/memstat.h"

int main(int argc, char **argv) {
    FILE *output = fopen("output.txt", "w");
    if (!output) {
        perror("fopen output.txt");
        return 1;
    }
    list db = {NULL, 0};
    char line[1024];

    if (argc == 2) {
        FILE *input = fopen(argv[1], "r");
        if (!input) { perror("fopen input"); return 1; }

        while (fgets(line, sizeof(line), input)) {
            line[strcspn(line, "\n")] = '\0';
            if (!strlen(line)) continue;
            execute_command(line, &db, output);
        }
        fclose(input);
    }

    while (1) {
        printf(":> "); fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;

        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;
        if (!strcmp(line, "exit")) break;

        execute_command(line, &db, output);
    }

    free_list(&db);          // <-- освобождение всей памяти
    write_memstat();
    fclose(output);
    return 0;
}