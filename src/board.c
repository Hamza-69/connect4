#define _XOPEN_SOURCE 500 // For usleep
#include "../include/board.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// --- ANSI Colors ---
#define C_RESET  "\033[0m"
#define C_RED    "\033[1;31m" 
#define C_YEL    "\033[1;33m" 
#define C_BLUE   "\033[1;34m" 
#define C_CLS    "\033[H\033[J"

void SetupBoard(char** arr) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            arr[i][j] = '.';
        }
    }
}

void PrintBoard(char **arr) {
    printf(C_CLS);
    printf("\n");
    printf("   ");
    for (int i = 1; i <= COLS; i++) printf(" %d  ", i);
    printf("\n");
    printf("  " C_BLUE "+---" "+---" "+---" "+---" "+---" "+---" "+---+" C_RESET "\n");

    for (int i = 0; i < ROWS; i++) {
        printf("  " C_BLUE "|" C_RESET);
        for (int j = 0; j < COLS; j++) {
            char p = arr[i][j];
            if (p == 'A') printf(" " C_RED "O" C_RESET " " C_BLUE "|" C_RESET);
            else if (p == 'B') printf(" " C_YEL "O" C_RESET " " C_BLUE "|" C_RESET);
            else printf("   " C_BLUE "|" C_RESET);
        }
        printf("\n");
        printf("  " C_BLUE "+---" "+---" "+---" "+---" "+---" "+---" "+---+" C_RESET "\n");
    }
    printf("\n");
}

int CheckMove(int inp, char **arr) {
    if (inp < 1 || inp > COLS) return 0;
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

void RemovePiece(char** arr, int col) {
    // Find the top piece in this column and remove it
    for (int i = 0; i < ROWS; i++) {
        if (arr[i][col-1] != '.') {
            arr[i][col-1] = '.';
            return;
        }
    }
}

void AnimateDrop(char** arr, int col, char player) {
    int target_row = -1;
    int c_idx = col - 1;
    for (int i = ROWS - 1; i >= 0; i--) {
        if (arr[i][c_idx] == '.') {
            target_row = i;
            break;
        }
    }
    if (target_row == -1) return;

    for (int r = 0; r <= target_row; r++) {
        arr[r][c_idx] = player;
        PrintBoard(arr);
        if (r < target_row) {
            usleep(40000); // Slightly faster animation
            arr[r][c_idx] = '.';
        }
    }
}

// --- History Implementation ---

void InitHistory(GameHistory* h) {
    h->top = 0;
    h->redoTop = 0;
}

void RecordMove(GameHistory* h, int col) {
    if (h->top < MAX_MOVES) {
        h->moveStack[h->top++] = col;
    }
    // Clear redo stack on new move
    h->redoTop = 0;
}

int PerformUndo(GameHistory* h, char** board) {
    if (h->top <= 0) return -1; // Empty
    
    int col = h->moveStack[--h->top];
    RemovePiece(board, col);
    
    // Add to Redo stack
    h->redoStack[h->redoTop++] = col;
    return col;
}

int PerformRedo(GameHistory* h, char** board, char player) {
    if (h->redoTop <= 0) return -1;
    
    int col = h->redoStack[--h->redoTop];
    MakeMove(board, col, player);
    h->moveStack[h->top++] = col;
    return col;
}