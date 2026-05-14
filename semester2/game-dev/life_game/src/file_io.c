#include "file_io.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>

int save_world_to_file(int world[HEIGHT][WIDTH], const char* filename) {
    FILE* file = fopen(filename, "w");
    if(!file) return 0;
    
    // Записываем размеры
    fprintf(file, "%d %d\n", WIDTH, HEIGHT);
    
    // Записываем поле
    for(int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++) {
            fprintf(file, "%c", world[y][x] ? 'X' : '.');
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    return 1;
}

int load_world_from_file(int world[HEIGHT][WIDTH], const char* filename) {
    FILE* file = fopen(filename, "r");
    if(!file) return 0;
    
    int file_width, file_height;
    fscanf(file, "%d %d\n", &file_width, &file_height);
    
    // Очищаем мир
    for(int y = 0; y < HEIGHT; y++)
        for(int x = 0; x < WIDTH; x++)
            world[y][x] = 0;
    
    // Загружаем (если размеры не совпадают, загружаем в центр)
    int offset_y = (HEIGHT - file_height) / 2;
    int offset_x = (WIDTH - file_width) / 2;
    
    char ch;
    for(int y = 0; y < file_height && y < HEIGHT; y++) {
        for(int x = 0; x < file_width && x < WIDTH; x++) {
            ch = fgetc(file);
            if(ch == 'X') {
                int world_y = y + offset_y;
                int world_x = x + offset_x;
                if(world_y >= 0 && world_y < HEIGHT && world_x >= 0 && world_x < WIDTH)
                    world[world_y][world_x] = 1;
            }
        }
        fgetc(file); // пропускаем символ новой строки
    }
    
    fclose(file);
    return 1;
}