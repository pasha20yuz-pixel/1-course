#include "world.h"
#include <stdlib.h>
#include <string.h>

int count_neighbors(int world[HEIGHT][WIDTH], int y, int x) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int ny = y + dy;
            int nx = x + dx;
            if (ny >= 0 && ny < HEIGHT && nx >= 0 && nx < WIDTH)
                count += world[ny][nx];
        }
    }
    return count;
}

void next_generation(int world[HEIGHT][WIDTH]) {
    int new_world[HEIGHT][WIDTH];
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int neighbors = count_neighbors(world, y, x);
            if (world[y][x] == 1)
                new_world[y][x] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            else
                new_world[y][x] = (neighbors == 3) ? 1 : 0;
        }
    }
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            world[y][x] = new_world[y][x];
}

int count_live_cells_static(int world[HEIGHT][WIDTH]) {
    int cnt = 0;
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            cnt += world[y][x];
    return cnt;
}

// === Функции с барьерами ===
int count_neighbors_with_barriers(int **world, Barriers *b, int width, int height, int y, int x) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int ny = y + dy;
            int nx = x + dx;
            if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                if (b && has_barrier_between(b, y, x, ny, nx)) continue;
                count += world[ny][nx];
            }
        }
    }
    return count;
}

void next_generation_with_barriers(int **world, Barriers *b, int width, int height, int *generation) {
    int **new_world = (int**)malloc(height * sizeof(int*));
    for (int i = 0; i < height; i++) {
        new_world[i] = (int*)calloc(width, sizeof(int));
    }
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int neighbors = count_neighbors_with_barriers(world, b, width, height, y, x);
            if (world[y][x] == 1)
                new_world[y][x] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            else
                new_world[y][x] = (neighbors == 3) ? 1 : 0;
        }
    }
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            world[y][x] = new_world[y][x];
        }
        free(new_world[y]);
    }
    free(new_world);
    if (generation) (*generation)++;
}