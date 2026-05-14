#include "memstat.h"
#include "parser.c"   // включает ast.c через #include "ast.c"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static AstNode* current_tree = NULL;

static void trim(char* s) {
    char* p = s;
    int l = strlen(p);
    while (isspace(p[l-1])) p[--l] = 0;
    while (*p && isspace(*p)) ++p, --l;
    memmove(s, p, l+1);
}

static int parse_variables(const char* arg_str, Variable* vars, int max_vars) {
    char buf[1024];
    strcpy(buf, arg_str);
    char* token = strtok(buf, " \t");
    int count = 0;
    while (token && count < max_vars) {
        char* eq = strchr(token, '=');
        if (!eq) return -1;
        *eq = '\0';
        strncpy(vars[count].name, token, 63);
        vars[count].name[63] = '\0';
        vars[count].value = atoll(eq+1);
        count++;
        token = strtok(NULL, " \t");
    }
    return count;
}

void process_command(const char* line) {
    char cmd_line[4096];
    strcpy(cmd_line, line);
    trim(cmd_line);
    if (strlen(cmd_line) == 0) return;

    char* cmd = strtok(cmd_line, " \t");
    char* args = strtok(NULL, "");

    if (strcmp(cmd, "parse") == 0) {
        if (!args) { printf("incorrect\n"); return; }
        int err = 0;
        AstNode* new_tree = parse_infix(args, &err);
        if (!err && new_tree) {
            if (current_tree) free_ast(current_tree);
            current_tree = new_tree;
            printf("success\n");
        } else {
            if (new_tree) free_ast(new_tree);
            printf("incorrect\n");
        }
    }
    else if (strcmp(cmd, "load_prf") == 0) {
        if (!args) { printf("incorrect\n"); return; }
        int err = 0;
        AstNode* new_tree = parse_prefix(args, &err);
        if (!err && new_tree) {
            if (current_tree) free_ast(current_tree);
            current_tree = new_tree;
            printf("success\n");
        } else {
            if (new_tree) free_ast(new_tree);
            printf("incorrect\n");
        }
    }
    else if (strcmp(cmd, "load_pst") == 0) {
        if (!args) { printf("incorrect\n"); return; }
        int err = 0;
        AstNode* new_tree = parse_postfix(args, &err);
        if (!err && new_tree) {
            if (current_tree) free_ast(current_tree);
            current_tree = new_tree;
            printf("success\n");
        } else {
            if (new_tree) free_ast(new_tree);
            printf("incorrect\n");
        }
    }
    else if (strcmp(cmd, "save_prf") == 0) {
        if (!current_tree) { printf("not_loaded\n"); return; }
        char buffer[4096];
        to_prefix(current_tree, buffer, sizeof(buffer));
        printf("%s\n", buffer);
    }
    else if (strcmp(cmd, "save_pst") == 0) {
        if (!current_tree) { printf("not_loaded\n"); return; }
        char buffer[4096];
        to_postfix(current_tree, buffer, sizeof(buffer));
        printf("%s\n", buffer);
    }
    else if (strcmp(cmd, "eval") == 0) {
        if (!current_tree) { printf("not_loaded\n"); return; }
        if (!args) {
            int err = 0;
            long long res = evaluate(current_tree, NULL, 0, &err);
            if (err) printf("incorrect\n");
            else printf("%lld\n", res);
            return;
        }
        Variable vars[64];
        int var_count = parse_variables(args, vars, 64);
        if (var_count < 0) { printf("incorrect\n"); return; }
        int err = 0;
        long long res = evaluate(current_tree, vars, var_count, &err);
        if (err) printf("incorrect\n");
        else printf("%lld\n", res);
    }
    else if (strcmp(cmd, "exit") == 0) {
        // nothing, will break in main
    }
    else {
        printf("incorrect\n");
    }
}

// ---------- статистика памяти ----------
MemStats stats = {0};

void memstat_print(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "malloc: %zu\n", stats.malloc_count);
    fprintf(f, "calloc: %zu\n", stats.calloc_count);
    fprintf(f, "realloc: %zu\n", stats.realloc_count);
    fprintf(f, "free: %zu\n", stats.free_count);
    fclose(f);
}

int main(int argc, char* argv[]) {
    FILE* input = stdin;
    if (argc > 1) {
        input = fopen("input.txt", "r");
        if (!input) {
            perror("input.txt");
            return 1;
        }
    }
    char line[4096];
    while (fgets(line, sizeof(line), input)) {
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, "exit") == 0) break;
        process_command(line);
    }
    if (input != stdin) fclose(input);
    if (current_tree) free_ast(current_tree);
    memstat_print("memstat.txt");
    return 0;
}