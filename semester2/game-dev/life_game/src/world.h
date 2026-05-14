#ifndef WORLD_H
#define WORLD_H

#include "barriers.h"

#define WIDTH 25
#define HEIGHT 25

// Классические функции (без барьеров)
int count_neighbors(int world[HEIGHT][WIDTH], int y, int x);
void next_generation(int world[HEIGHT][WIDTH]);

// Функции с барьерами (для динамических миров, используемых в уровнях)
int count_neighbors_with_barriers(int **world, Barriers *b, int width, int height, int y, int x);
void next_generation_with_barriers(int **world, Barriers *b, int width, int height, int *generation);

// Подсчёт живых клеток (для статического мира)
int count_live_cells_static(int world[HEIGHT][WIDTH]);

#endif