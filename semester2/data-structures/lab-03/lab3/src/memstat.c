#include <stdio.h>
#include "../include/memstat.h"

memstat stats = {0, 0, 0, 0};

void write_memstat(void) {
    FILE *f = fopen("memstat.txt", "w");
    if (!f) { perror("memstat.txt"); return; }

    fprintf(f, "malloc:%zu\n",  stats.malloc_count);
    fprintf(f, "calloc:%zu\n",  stats.calloc_count);
    fprintf(f, "realloc:%zu\n", stats.realloc_count);
    fprintf(f, "free:%zu\n",    stats.free_count);

    fclose(f);
}