#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include <stddef.h>

unsigned int SDBMHash(const char* str, unsigned int length);
unsigned int djb2(const char* str, unsigned int length);
unsigned int adler32(const char* str, unsigned int length);

#endif