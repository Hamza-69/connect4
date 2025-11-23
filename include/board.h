#ifndef BOARD_H
#define BOARD_H

#define ROWS 6
#define COLS 7
#define MAX_MOVES (ROWS * COLS)

// History structure for Undo/Redo
typedef struct {
    int moveStack[MAX_MOVES];
    int top; // Index of the next empty slot
    int redoStack[MAX_MOVES];
    int redoTop;
} GameHistory;

void SetupBoard(char** arr);
void PrintBoard(char** arr);
int CheckMove(int inp, char** arr);
void MakeMove(char** arr, int inp, char letter);
void AnimateDrop(char** arr, int col, char player);
void RemovePiece(char** arr, int col); // New helper for Undo

// History Functions
void InitHistory(GameHistory* h);
void RecordMove(GameHistory* h, int col);
int PerformUndo(GameHistory* h, char** board); // Returns col undone, or -1
int PerformRedo(GameHistory* h, char** board, char player); // Returns col redone, or -1

#endif