#ifndef BOARD_H
#define BOARD_H

#define ROWS 6
#define COLS 7

void SetupBoard(char** arr);
void PrintBoard(char** arr);
int CheckMove(int inp, char** arr);
void MakeMove(char** arr, int inp, char letter);

#endif