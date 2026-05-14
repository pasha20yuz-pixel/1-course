#include "memory.h"
#include <stdio.h>

int malloc_count = 0;
int calloc_count = 0;
int realloc_count = 0;
int free_count = 0;

void* my_malloc(size_t size) {
    malloc_count++;
    return malloc(size);
}

void* my_calloc(size_t nmemb, size_t size) {
    calloc_count++;
    return calloc(nmemb, size);
}

void* my_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        malloc_count++;
    } else {
        realloc_count++;
    }
    return realloc(ptr, size);
}

void my_free(void *ptr) {
    if (ptr) {
        free_count++;
        free(ptr);
    }
}

void write_memstat(void) {
    FILE *f = fopen("memstat.txt", "w");
    if (f) {
        fprintf(f, "malloc:%d\n", malloc_count);
        fprintf(f, "calloc:%d\n", calloc_count);
        fprintf(f, "realloc:%d\n", realloc_count);
        fprintf(f, "free:%d\n", free_count);
        fclose(f);
    }
}