#include "../include/board.h"
#include <stdio.h>

void SetupBoard(char** arr) {
  for (int i = 0; i<ROWS; i++) {
    for (int j = 0; j<COLS; j++) {
      arr[i][j] = '.';
    }
  }
}

void PrintBoard(char **arr) {
  for (int i = 0; i <  ROWS; i++) {
    for (int j = 0; j < COLS; j++){
      printf("%c ", arr[i][j]);
    }
    printf("\n");
  }
  for (int i = 1; i <=COLS; i++) {
    printf("%d ", i);
  }
  printf("\n");
}

int CheckMove(int inp, char **arr) {
  return arr[0][inp-1] == '.'; 
}

void MakeMove(char **arr, int inp, char letter) {
  for (int i = ROWS-1; i >= 0; i--) {
    if (arr[i][inp-1] == '.') {
      arr[i][inp-1] = letter;
      return;
    }
  }
}