#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "memory.h"
#include "parser.h"

int main() {
    FILE *out = fopen("output.txt", "w");
    if (!out) {
        fprintf(stderr, "Cannot open output.txt\n");
        return 1;
    }

    FILE *in = fopen("input.txt", "r");
    if (in) {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), in)) {
            size_t len = strlen(buffer);
            while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
                buffer[--len] = '\0';
            process_command(out, buffer);
        }
        fclose(in);
    }

    printf("Enter commands (Ctrl+Z to finish):\n");
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), stdin)) {
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
            buffer[--len] = '\0';
        process_command(out, buffer);
    }

    fclose(out);
    free_database();
    write_memstat();
    return 0;
}