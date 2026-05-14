#include <stdio.h>

void compound_interest(int P, int R, int T){
    float sum = 1.0 + (float)R/100;
    float power = 1.0;
    for (int i = 0; i < T; i++){
        power = power * sum;
    }
    float c_i = P * power;
    printf("%f", c_i);
}

int main(){
    int P, R, T;
    scanf("%d %d %d", &P, &R, &T);
    
    compound_interest(P, R, T);

    return 0;
}