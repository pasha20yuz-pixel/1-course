#include <stdio.h>

int factorial(int number_fact) {
    if (number_fact==1 || number_fact==0 ){
        return 1;
    }
    int res_fact = 1;
    for (int i = 1; i <= number_fact; i++) {
        res_fact *= i;
    }
    return res_fact;
}

int Combination(int n, int k) {
    return (factorial(n) / (factorial(k) * factorial(n - k)));
}

void Pascal_Triangle(int rows) {
    for (int i = 0; i < rows; i++) {
        // Вывод пробелов для центрирования
        for (int space = 0; space < rows - i + 1; space++) {
            printf("  ");
        }
        
        // Вывод чисел в строке
        for (int j = 0; j <= i; j++) {
            printf("%3d ", Combination(i, j));
        }
        printf("\n");
    }
}

int main() {
    int rows;
    
    scanf("%d", &rows);
    Pascal_Triangle(rows);
    return 0;
}