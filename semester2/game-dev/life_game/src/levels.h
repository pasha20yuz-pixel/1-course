#ifndef LEVELS_H
#define LEVELS_H

#include "barriers.h"

// Union для дополнительных параметров уровня (пока не используется, но для соответствия требованию)
typedef union {
    int int_param;
    float float_param;
    char string_param[64];
} LevelExtra;

typedef struct {
    int id;
    int width, height;
    int max_start_cells;
    int target_generations;
    Barriers *barriers;
    char description[256];
    LevelExtra extra;   // демонстрация union
} Level;

Level* load_level(int level_id);
void free_level(Level *l);
int get_total_levels();

#endif