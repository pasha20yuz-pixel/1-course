#include <stdio.h>
#include <string.h>

int main() {
    char input[100];
    char words[50][50];
    int word_count = 0;
    int char_index = 0;
    
    fgets(input, 100, stdin);
    
    int i = 0;
    while (input[i] != '\n' && input[i] != '\0') {
        i++;
    }
    input[i] = '\0';
    
    i = 0;
    while (input[i] != '\0') {
        if (input[i] == ' ') {
            i++;
            continue;
        }
        
        char_index = 0;
        
        while (input[i] != '\0' && input[i] != ' ') {
            words[word_count][char_index] = input[i];
            char_index++;
            i++;
        }
        
        words[word_count][char_index] = '\0';
        word_count++;
    }
    
    for (int j = word_count - 1; j >= 0; j--) {
        printf("%s", words[j]);
        if (j > 0) {
            printf(" ");
        }
    }
    printf("\n");
    
    return 0;
}