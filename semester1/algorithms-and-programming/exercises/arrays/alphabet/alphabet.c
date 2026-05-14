#include <stdio.h>
#include <ctype.h>

int main(){
    char letters[5];
    int temp;
    for (int i = 0; i < 5; i++){
        scanf("%c", &letters[i]);
    }

    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4 - i; j++) {
            if(letters[j] > letters[j + 1]) {
                temp = letters[j];
                letters[j] = letters[j + 1];
                letters[j + 1] = temp;
            }
        }
    }
    
    for(int i = 0; i < 5; i++) {
        printf("%c ", letters[i]);
    }
    printf("\n");
    
    return 0;

}