#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "lcg_period.h"

void print_usage(const char *progname) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s a x0 c m          - compute period for single x0\n", progname);
    fprintf(stderr, "  %s -f filename       - read multiple lines from file (format: a x0 c m per line)\n", progname);
    fprintf(stderr, "  %s -                 - read from stdin (same format)\n", progname);
}

int parse_uint64(const char *str, uint64_t *out) {
    char *endptr;
    errno = 0;
    unsigned long long val = strtoull(str, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || val > UINT64_MAX) {
        return 0;
    }
    *out = (uint64_t)val;
    return 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // Режим чтения из файла или stdin
    if (strcmp(argv[1], "-f") == 0 && argc >= 3) {
        FILE *f = fopen(argv[2], "r");
        if (!f) {
            perror("fopen");
            return 1;
        }
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            uint64_t a, x0, c, m;
            if (sscanf(line, "%llu %llu %llu %llu", &a, &x0, &c, &m) == 4) {
                uint64_t period = find_period(a, x0, c, m);
                printf("%llu %llu %llu %llu -> %llu\n", a, x0, c, m, period);
            }
        }
        fclose(f);
        return 0;
    }
    else if (strcmp(argv[1], "-") == 0) {
        // Чтение из stdin
        char line[256];
        while (fgets(line, sizeof(line), stdin)) {
            uint64_t a, x0, c, m;
            if (sscanf(line, "%llu %llu %llu %llu", &a, &x0, &c, &m) == 4) {
                uint64_t period = find_period(a, x0, c, m);
                printf("%llu %llu %llu %llu -> %llu\n", a, x0, c, m, period);
            }
        }
        return 0;
    }
    else if (argc == 5) {
        // Одиночный вызов: a x0 c m
        uint64_t a, x0, c, m;
        if (parse_uint64(argv[1], &a) &&
            parse_uint64(argv[2], &x0) &&
            parse_uint64(argv[3], &c) &&
            parse_uint64(argv[4], &m)) {
            uint64_t period = find_period(a, x0, c, m);
            printf("%llu\n", period);
            return 0;
        } else {
            fprintf(stderr, "Invalid number format\n");
            return 1;
        }
    }
    else {
        print_usage(argv[0]);
        return 1;
    }
}