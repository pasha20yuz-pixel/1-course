#include "hash_functions.h"

unsigned int SDBMHash(const char* str, unsigned int length) {
    unsigned int hash = 0;
    for (unsigned int i = 0; i < length; ++i, ++str) {
        hash = (*str) + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

unsigned int djb2(const char* str, unsigned int length) {
    unsigned int hash = 5381;
    for (unsigned int i = 0; i < length; ++i, ++str) {
        hash = ((hash << 5) + hash) + (*str);
    }
    return hash;
}

unsigned int adler32(const char* str, unsigned int length) {
    unsigned int s1 = 1;
    unsigned int s2 = 0;
    for (unsigned int i = 0; i < length; ++i, ++str) {
        s1 = (s1 + (*str)) % 65521;
        s2 = (s2 + s1) % 65521;
    }
    return (s2 << 16) | s1;
}