#ifndef BARRIERS_H
#define BARRIERS_H

#include <stdbool.h>

typedef struct {
    int width;
    int height;
    bool **h;   // горизонтальные барьеры между (y,x) и (y,x+1)  размер [height][width-1]
    bool **v;   // вертикальные барьеры   между (y,x) и (y+1,x)  размер [height-1][width]
} Barriers;

Barriers* create_barriers(int width, int height);
void free_barriers(Barriers *b);
void set_h_barrier(Barriers *b, int y, int x, bool val);
void set_v_barrier(Barriers *b, int y, int x, bool val);
bool has_h_barrier(Barriers *b, int y, int x);
bool has_v_barrier(Barriers *b, int y, int x);
bool has_barrier_between(Barriers *b, int y1, int x1, int y2, int x2);

#endif