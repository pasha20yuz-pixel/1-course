#include <stdio.h>

int main(){
    char filename[2000];
    FILE *file;

    scanf("%s", &filename);
    file = fopen(filename, "r");
    if (file == NULL) return 1;

    int count[256] = {0};
    int max_count = 0;
    char max_char;
    int ch;

    while ((ch = fgetc(file)) != EOF){
        count[ch]++;
        if (count[ch] > max_count){
            max_count = count[ch];
            max_char = ch;
        }
    }

    fclose(file);
    
    printf("%c: %d", max_char, max_count);

    return 0;
}