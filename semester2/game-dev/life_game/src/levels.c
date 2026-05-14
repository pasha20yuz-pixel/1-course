#include "levels.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Level* load_level(int level_id) {
    char filename[64];
    snprintf(filename, sizeof(filename), "levels/level%d.txt", level_id);
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;
    
    Level *l = (Level*)malloc(sizeof(Level));
    l->id = level_id;
    fscanf(f, "%d %d", &l->width, &l->height);
    fscanf(f, "%d %d", &l->max_start_cells, &l->target_generations);
    fgetc(f); // пропустить newline
    
    l->barriers = create_barriers(l->width, l->height);
    
    // Читаем горизонтальные барьеры (height строк, каждая width-1 символов)
    for (int y = 0; y < l->height; y++) {
        char line[256];
        fgets(line, sizeof(line), f);
        for (int x = 0; x < l->width - 1; x++) {
            if (line[x] == '1') set_h_barrier(l->barriers, y, x, true);
        }
    }
    // Читаем вертикальные барьеры (height-1 строк, каждая width символов)
    for (int y = 0; y < l->height - 1; y++) {
        char line[256];
        fgets(line, sizeof(line), f);
        for (int x = 0; x < l->width; x++) {
            if (line[x] == '1') set_v_barrier(l->barriers, y, x, true);
        }
    }
    // Описание
    fgets(l->description, sizeof(l->description), f);
    size_t len = strlen(l->description);
    if (len > 0 && l->description[len-1] == '\n') l->description[len-1] = '\0';
    
    fclose(f);
    return l;
}

void free_level(Level *l) {
    if (l) {
        free_barriers(l->barriers);
        free(l);
    }
}

int get_total_levels() {
    return 10;
}