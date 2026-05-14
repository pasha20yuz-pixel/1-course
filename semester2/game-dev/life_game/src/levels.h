#ifndef LEVELS_H
#define LEVELS_H

#include "barriers.h"

typedef struct {
    int id;
    int width, height;
    int max_start_cells;
    int target_generations;
    Barriers *barriers;
    char description[256];
} Level;

Level* load_level(int level_id);
void free_level(Level *l);
int get_total_levels();

#endif