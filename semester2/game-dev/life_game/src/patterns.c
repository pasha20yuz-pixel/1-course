#include "patterns.h"
#include <stdlib.h>
#include <time.h>

void pattern_glider(int world[HEIGHT][WIDTH], int start_y, int start_x) {
    int glider[5][2] = {{0,1}, {1,2}, {2,0}, {2,1}, {2,2}};
    for (int i = 0; i < 5; i++) {
        int y = start_y + glider[i][0];
        int x = start_x + glider[i][1];
        if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH)
            world[y][x] = 1;
    }
}

void pattern_blinker(int world[HEIGHT][WIDTH], int start_y, int start_x) {
    for (int i = 0; i < 3; i++) {
        int y = start_y;
        int x = start_x + i;
        if (x < WIDTH) world[y][x] = 1;
    }
}

void pattern_random(int world[HEIGHT][WIDTH], int density_percent) {
    srand(time(NULL));
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            world[y][x] = (rand() % 100 < density_percent) ? 1 : 0;
}