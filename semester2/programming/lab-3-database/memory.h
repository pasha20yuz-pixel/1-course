#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>

extern int malloc_count;
extern int calloc_count;
extern int realloc_count;
extern int free_count;

void* my_malloc(size_t size);
void* my_calloc(size_t nmemb, size_t size);
void* my_realloc(void *ptr, size_t size);
void my_free(void *ptr);
void write_memstat(void);

#endif