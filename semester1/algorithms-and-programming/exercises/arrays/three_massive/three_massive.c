#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int set_min_max(int arr[], int size, int *min, int *max, int *i_min, int *i_max){
    *min = arr[0];
    *max = arr[0];
    *i_min = 0;
    *i_max = 0;
    
    for (int i = 0; i < size; i++){
        if (arr[i] < *min){
            *min = arr[i];
            *i_min = i;
        }
        if (arr[i] > *max){
            *max = arr[i];
            *i_max = i;
        }
    }
} 

int main(){
    srand(time(0));
    const int SIZE = 10;
    int arr1[SIZE], arr2[SIZE], arr3[SIZE];

    for (int i = 0; i < SIZE; i++){
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
    }

    int min, max, i_min, i_max;
    for (int i = 0; i < SIZE; i++){
        printf("%d ", arr1[i]);
    }
    set_min_max(arr1, SIZE, &min, &max, &i_min, &i_max);
    printf("\nmin = %d\nmax = %d\ni_min = %d\ni_max = %d\n", min, max, i_min, i_max);

    for (int i = 0; i < SIZE; i++){
        printf("%d ", arr2[i]);
    }
    set_min_max(arr2, SIZE, &min, &max, &i_min, &i_max);
    printf("\nmin = %d\nmax = %d\ni_min = %d\ni_max = %d\n", min, max, i_min, i_max);

    for (int i = 0; i < SIZE; i++){
        printf("%d ", arr3[i]);
    }
    set_min_max(arr3, SIZE, &min, &max, &i_min, &i_max);
    printf("\nmin = %d\nmax = %d\ni_min = %d\ni_max = %d\n", min, max, i_min, i_max);

    return 0;

}