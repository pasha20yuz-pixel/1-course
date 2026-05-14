#ifndef _MEMSTAT_H_
#define _MEMSTAT_H_

#include "types.h"

typedef struct
{
    size_t malloc_count;
    size_t calloc_count;
    size_t realloc_count;
    size_t free_count;
} memstat;

extern memstat stats;

void write_memstat(void);

#define MALLOC(size)       (stats.malloc_count++,  malloc(size))
#define CALLOC(n, size)    (stats.calloc_count++,  calloc(n, size))
#define FREE(ptr)          (stats.free_count++,    free(ptr))
#define REALLOC(ptr, size) ((ptr) == NULL \
    ? (stats.malloc_count++,  realloc(NULL, size)) \
    : (stats.realloc_count++, realloc(ptr,  size)))

#endif