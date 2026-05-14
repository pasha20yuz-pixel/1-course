#include <stdio.h>

void reverse(char *str){
    if (str == NULL) return;

    char *end = str;
    while(*end != '\0'){
        end++;
    }
    end--;

    char *start = str;
    while (start < end){
        char t = *start;
        *start = *end;
        *end = t;

        start++;
        end--;
    }
}
int main(){
    char string[6];

    scanf("%s", string);
    reverse(string);
    printf("%s", string);
    return 0;
}