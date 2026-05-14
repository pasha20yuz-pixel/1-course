#ifndef MEMSTAT_H
#define MEMSTAT_H

#include <stdlib.h>

typedef struct {
    size_t malloc_count;
    size_t calloc_count;
    size_t realloc_count;
    size_t free_count;
} MemStats;

extern MemStats stats;

#define MALLOC(size)     (stats.malloc_count++, malloc(size))
#define CALLOC(n, size)  (stats.calloc_count++, calloc(n, size))
#define REALLOC(ptr, sz) (stats.realloc_count++, realloc(ptr, sz))
#define FREE(ptr)        (stats.free_count++, free(ptr))

void memstat_print(const char* filename);

#endif