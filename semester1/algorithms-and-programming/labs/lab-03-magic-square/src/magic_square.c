#include <stdio.h>

void magic_square(int square[3][3]){
    for (int i = 0; i<3; i++){
        for (int j = 0; j < 3; j++){
            square[i][j] = 0;
        }
    }
    int row = 0;
    int col = 1;

    for (int num = 1; num <= 9; num++){
        square[row][col] = num;

        int old_row = row;
        int old_col = col;

        row = (row - 1 + 3) % 3;
        col = (col + 1) % 3;

        if (square[row][col] != 0) {
            row = (old_row + 1) % 3;
            col = old_col;
        }
    }

    for (int r = 0; r < 3; r++){
        for (int c = 0; c < 3; c++){
            printf("%2d ", square[r][c]);
        }
        printf("\n");
    }
}


int main(){
    int square[3][3];
    magic_square(square);
    return  0;
}