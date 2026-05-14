#ifndef FILE_IO_H
#define FILE_IO_H

#include "world.h"

// Сохраняет текущее состояние в файл
int save_world_to_file(int world[HEIGHT][WIDTH], const char* filename);

// Загружает мир из файла (формат: первая строка WIDTH HEIGHT, затем 'X' и '.')
int load_world_from_file(int world[HEIGHT][WIDTH], const char* filename);

// Показывает список доступных паттернов в папке patterns/
void list_available_patterns();

#endif