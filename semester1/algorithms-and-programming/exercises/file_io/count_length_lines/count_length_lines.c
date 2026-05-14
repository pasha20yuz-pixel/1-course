#include <stdio.h>

int main(){
    char filename[1000];
    FILE *file;

    scanf("%s", &filename);

    file = fopen(filename, "r");
    if (file == NULL) return 1;

    int length_count[31] = {0}; // индексы 0..30
    int current_len = 0;
    int ch;
    int in_word = 0;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n' || ch == '\r') {
            if (in_word) {
                if (current_len <= 30) {
                    length_count[current_len]++;
                }
                current_len = 0;
                in_word = 0;
            }
        } else {
            current_len++;
            in_word = 1;
        }
    }

    // Обработка последней строки
    if (in_word && current_len <= 30) {
        length_count[current_len]++;
    }

    fclose(file);

    for (int i = 1; i < 31; i++){
        printf("%d %d\n", i, length_count[i]);
    }
        

    return 0;
}