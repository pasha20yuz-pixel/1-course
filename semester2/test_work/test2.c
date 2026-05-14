#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

int my_strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

const char* get_translit(unsigned char first, unsigned char second) {
    if (first == 0xD0 && second == 0xB9) return "y";
    if (first == 0xD1 && second == 0x86) return "ts";
    if (first == 0xD1 && second == 0x83) return "u";
    if (first == 0xD0 && second == 0xBA) return "k";
    if (first == 0xD0 && second == 0xB5) return "e";
    if (first == 0xD0 && second == 0xBD) return "n";
    if (first == 0xD0 && second == 0xB3) return "g";
    if (first == 0xD1 && second == 0x88) return "sh";
    if (first == 0xD1 && second == 0x89) return "shch";
    if (first == 0xD0 && second == 0xB7) return "z";
    if (first == 0xD1 && second == 0x85) return "kh";
    if (first == 0xD1 && second == 0x8A) return "'";
    if (first == 0xD1 && second == 0x91) return "yo";
    if (first == 0xD1 && second == 0x84) return "f";
    if (first == 0xD1 && second == 0x8B) return "y";
    if (first == 0xD0 && second == 0xB2) return "v";
    if (first == 0xD0 && second == 0xB0) return "a";
    if (first == 0xD0 && second == 0xBF) return "p";
    if (first == 0xD1 && second == 0x80) return "r";
    if (first == 0xD0 && second == 0xBE) return "o";
    if (first == 0xD0 && second == 0xBB) return "l";
    if (first == 0xD0 && second == 0xB4) return "d";
    if (first == 0xD0 && second == 0xB6) return "zh";
    if (first == 0xD1 && second == 0x8D) return "e";
    if (first == 0xD1 && second == 0x8F) return "ya";
    if (first == 0xD1 && second == 0x87) return "ch";
    if (first == 0xD1 && second == 0x81) return "s";
    if (first == 0xD0 && second == 0xBC) return "m";
    if (first == 0xD0 && second == 0xB8) return "i";
    if (first == 0xD1 && second == 0x82) return "t";
    if (first == 0xD1 && second == 0x8C) return "'";
    if (first == 0xD0 && second == 0xB1) return "b";
    if (first == 0xD1 && second == 0x8E) return "yu";

    if (first == 0xD0 && second == 0x99) return "Y";
    if (first == 0xD0 && second == 0xA6) return "Ts";
    if (first == 0xD0 && second == 0xA3) return "U";
    if (first == 0xD0 && second == 0x9A) return "K";
    if (first == 0xD0 && second == 0x95) return "E";
    if (first == 0xD0 && second == 0x9D) return "N";
    if (first == 0xD0 && second == 0x93) return "G";
    if (first == 0xD0 && second == 0xA8) return "Sh";
    if (first == 0xD0 && second == 0xA9) return "Shch";
    if (first == 0xD0 && second == 0x97) return "Z";
    if (first == 0xD0 && second == 0xA5) return "Kh";
    if (first == 0xD0 && second == 0xAA) return "'";
    if (first == 0xD0 && second == 0x81) return "Yo";
    if (first == 0xD0 && second == 0xA4) return "F";
    if (first == 0xD0 && second == 0xAB) return "Y";
    if (first == 0xD0 && second == 0x92) return "V";
    if (first == 0xD0 && second == 0x90) return "A";
    if (first == 0xD0 && second == 0x9F) return "P";
    if (first == 0xD0 && second == 0xA0) return "R";
    if (first == 0xD0 && second == 0x9E) return "O";
    if (first == 0xD0 && second == 0x9B) return "L";
    if (first == 0xD0 && second == 0x94) return "D";
    if (first == 0xD0 && second == 0x96) return "Zh";
    if (first == 0xD0 && second == 0xAD) return "E";
    if (first == 0xD0 && second == 0xAF) return "Ya";
    if (first == 0xD0 && second == 0xA7) return "Ch";
    if (first == 0xD0 && second == 0xA1) return "S";
    if (first == 0xD0 && second == 0x9C) return "M";
    if (first == 0xD0 && second == 0x98) return "I";
    if (first == 0xD0 && second == 0xA2) return "T";
    if (first == 0xD0 && second == 0xAC) return "'";
    if (first == 0xD0 && second == 0x91) return "B";
    if (first == 0xD0 && second == 0xAE) return "Yu";

    return NULL;
}

char* ConvertRussian(char const* str) {
    if (!str) return NULL;

    int len = 0;
    const unsigned char* p = (const unsigned char*)str;

    while (*p) {
        if (*p >= 0x80 && p[1]) {
            const char* trans = get_translit(p[0], p[1]);
            if (trans) {
                len += my_strlen(trans);
            }
            p += 2;
        } else {
            len += 1;
            p += 1;
        }
    }

    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;

    char* out = result;
    p = (const unsigned char*)str;

    while (*p) {
        if (*p >= 0x80 && p[1]) {
            const char* trans = get_translit(p[0], p[1]);
            if (trans) {
                const char* t = trans;
                while (*t) {
                    *out++ = *t++;
                }
            }
            p += 2;
        } else {
            *out++ = *p;
            p += 1;
        }
    }

    *out = '\0';
    return result;
}

char* ReadLine(void) {
    int size = 100;
    int len = 0;
    char* buf = (char*)malloc(size);
    if (!buf) return NULL;

    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        if (len + 1 >= size) {
            size *= 2;
            char* new_buf = (char*)realloc(buf, size);
            if (!new_buf) {
                free(buf);
                return NULL;
            }
            buf = new_buf;
        }
        buf[len++] = c;
    }

    if (len == 0 && c == EOF) {
        free(buf);
        return NULL;
    }

    buf[len] = '\0';
    return buf;
}

int main() {
    system("chcp 65001 > nul");
    printf("Введите строки (пустая строка - выход):\n\n");

    while (1) {
        printf("> ");
        char* input = ReadLine();

        if (!input || input[0] == '\0') {
            free(input);
            printf("До свидания!\n");
            break;
        }

        char* result = ConvertRussian(input);
        if (result) {
            printf("%s\n\n", result);
            free(result);
        } else {
            printf("Ошибка!\n\n");
        }

        free(input);
    }

    return 0;
}