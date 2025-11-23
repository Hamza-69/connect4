#include "../include/board.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main() {
    char** board = malloc(ROWS * sizeof(char*));
    for (int i = 0; i < ROWS; i++) {
        board[i] = malloc(COLS * sizeof(char));
    }

    SetupBoard(board);

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            assert(board[r][c] == '.');
        }
    }

    assert(CheckMove(1, board) == 1);
    assert(CheckMove(7, board) == 1);
    assert(CheckMove(0, board) == 0);
    assert(CheckMove(8, board) == 0);

    for (int i = 0; i < ROWS; i++) free(board[i]);
    free(board);

    printf("test_board PASSED\n");
    return 0;
}