#include <stdio.h>

int main(){
    int num;
    scanf("%d", &num);

    while (num % 2 == 0) {
        printf("2 ");
        num = num / 2;
    }
    
    for (int i = 3; i * i <= num; i = i + 2) {
        while (num % i == 0) {
            printf("%d ", i);
            num = num / i;
        }
    }

    if (num > 1) {
        printf("%d", num);
    }

    printf("\n");
    return 0;
}