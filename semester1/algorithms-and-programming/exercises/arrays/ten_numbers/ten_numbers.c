#include <stdio.h>

int sum_arr(int *arr){
    int sum = 0;
    for (int i = 0; i < 10; i++){
        sum += arr[i];
    }
    return sum;
}

void product_arr(int *arr){
    int product = 1;
    for (int i = 0; i < 10; i++){
        product *= arr[i];
    }
    printf("Mult: %d\n", product);
}

void min_max_arr(int *arr){
    int min = arr[0];
    int max = arr[0];
    for (int i = 0; i < 10; i++){
        if (arr[i] < min){
            min = arr[i];
        }
    }
    for (int i = 0; i < 10; i++){
        if (arr[i] > max){
            max = arr[i];
        }
    }
    printf("Min: %d\nMax: %d\n", min, max);
}

void AVG_arr(int *arr){
    float avg;
    avg = sum_arr(arr) / 10.0;
    float diff;
    int diff_arr[10];

    for (int i = 0; i < 10; i++){
        if ((float)arr[i] >= avg){
            diff = (float)arr[i] - avg;
        }
        else {
            diff = -((float)arr[i] - avg);
        }
        diff_arr[i] = diff;
    }
    float min_diff = diff_arr[0];
    int flag = 0;
    for (int i = 0; i < 10; i++){
        if (diff_arr[i] < min_diff){
            min_diff = diff_arr[i];
            flag = i;
        }
    }
    printf("AVG: %f\nCloser: %d\n", avg, arr[flag]);
}

int main(){
    int arr[10];
    int result;
    for (int i = 0; i < 10; i++){
        scanf("%d", &arr[i]);
    }

    result = sum_arr(arr);
    printf("Sum: %d\n", result);
    product_arr(arr);
    min_max_arr(arr);
    AVG_arr(arr);

    return 0;
}