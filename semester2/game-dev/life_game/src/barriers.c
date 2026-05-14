#include "barriers.h"
#include <stdlib.h>
#include <string.h>

Barriers* create_barriers(int width, int height) {
    Barriers *b = (Barriers*)malloc(sizeof(Barriers));
    b->width = width;
    b->height = height;
    b->h = (bool**)malloc(height * sizeof(bool*));
    for (int y = 0; y < height; y++)
        b->h[y] = (bool*)calloc(width - 1, sizeof(bool));
    b->v = (bool**)malloc((height - 1) * sizeof(bool*));
    for (int y = 0; y < height - 1; y++)
        b->v[y] = (bool*)calloc(width, sizeof(bool));
    return b;
}

void free_barriers(Barriers *b) {
    if (!b) return;
    for (int y = 0; y < b->height; y++) free(b->h[y]);
    free(b->h);
    for (int y = 0; y < b->height - 1; y++) free(b->v[y]);
    free(b->v);
    free(b);
}

void set_h_barrier(Barriers *b, int y, int x, bool val) {
    if (y >= 0 && y < b->height && x >= 0 && x < b->width - 1)
        b->h[y][x] = val;
}
void set_v_barrier(Barriers *b, int y, int x, bool val) {
    if (y >= 0 && y < b->height - 1 && x >= 0 && x < b->width)
        b->v[y][x] = val;
}
bool has_h_barrier(Barriers *b, int y, int x) {
    return (y >= 0 && y < b->height && x >= 0 && x < b->width - 1) ? b->h[y][x] : false;
}
bool has_v_barrier(Barriers *b, int y, int x) {
    return (y >= 0 && y < b->height - 1 && x >= 0 && x < b->width) ? b->v[y][x] : false;
}

bool has_barrier_between(Barriers *b, int y1, int x1, int y2, int x2) {
    int dy = y2 - y1, dx = x2 - x1;
    if (dy == 0 && dx == 1)  // направо
        return has_h_barrier(b, y1, x1);
    if (dy == 0 && dx == -1) // налево
        return has_h_barrier(b, y1, x1-1);
    if (dy == 1 && dx == 0)  // вниз
        return has_v_barrier(b, y1, x1);
    if (dy == -1 && dx == 0) // вверх
        return has_v_barrier(b, y1-1, x1);
    // Диагонали не блокируются
    return false;
}