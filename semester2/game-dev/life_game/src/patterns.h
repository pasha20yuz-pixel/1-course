#ifndef PATTERNS_H
#define PATTERNS_H

#include "world.h"

void pattern_glider(int world[HEIGHT][WIDTH], int start_y, int start_x);
void pattern_blinker(int world[HEIGHT][WIDTH], int start_y, int start_x);
void pattern_random(int world[HEIGHT][WIDTH], int density_percent);

#endif