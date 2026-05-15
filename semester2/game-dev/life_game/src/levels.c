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
    if (!l) {
        fclose(f);
        return NULL;
    }
    
    // Чтение размеров и параметров
    if (fscanf(f, "%d %d", &l->width, &l->height) != 2) {
        free(l);
        fclose(f);
        return NULL;
    }
    if (fscanf(f, "%d %d", &l->max_start_cells, &l->target_generations) != 2) {
        free(l);
        fclose(f);
        return NULL;
    }
    fgetc(f); // пропустить символ новой строки
    
    // Создание барьеров
    l->barriers = create_barriers(l->width, l->height);
    if (!l->barriers) {
        free(l);
        fclose(f);
        return NULL;
    }
    
    // Чтение горизонтальных барьеров (height строк, каждая width-1 символов)
    for (int y = 0; y < l->height; y++) {
        char line[256];
        if (!fgets(line, sizeof(line), f)) {
            free_level(l);
            fclose(f);
            return NULL;
        }
        for (int x = 0; x < l->width - 1; x++) {
            if (line[x] == '1') set_h_barrier(l->barriers, y, x, true);
        }
    }
    
    // Чтение вертикальных барьеров (height-1 строк, каждая width символов)
    for (int y = 0; y < l->height - 1; y++) {
        char line[256];
        if (!fgets(line, sizeof(line), f)) {
            free_level(l);
            fclose(f);
            return NULL;
        }
        for (int x = 0; x < l->width; x++) {
            if (line[x] == '1') set_v_barrier(l->barriers, y, x, true);
        }
    }
    
    // Чтение описания
    if (!fgets(l->description, sizeof(l->description), f)) {
        free_level(l);
        fclose(f);
        return NULL;
    }
    // Удалить завершающий символ новой строки
    size_t len = strlen(l->description);
    if (len > 0 && l->description[len-1] == '\n') l->description[len-1] = '\0';
    
    // Инициализация union (по умолчанию нулём)
    l->extra.int_param = 0;
    l->extra.float_param = 0.0f;
    l->extra.string_param[0] = '\0';
    
    l->id = level_id;
    fclose(f);
    return l;
}

void free_level(Level *l) {
    if (l) {
        if (l->barriers) free_barriers(l->barriers);
        free(l);
    }
}

int get_total_levels() {
    return 10;
}