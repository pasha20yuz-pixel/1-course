#include <stdio.h>
#include <stdlib.h> 

int main() {
    int x, y, max_num, result;

    printf("Enter the first integer: ");
    scanf("%d", &x);

    printf("Enter the second integer: ");
    scanf("%d", &y);

    if (x > y) {
        max_num = x;
    } else {
        max_num = y;
    }

    result = max_num * 3;

    printf("OUTPUT\n");

    for (int i = 10; i <= result; i++) {
        printf("%d\n", i);
    }

    return 0;
}