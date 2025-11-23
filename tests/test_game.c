#include "../include/board.h"
#include "../include/game.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void setup(char*** board) {
    *board = malloc(ROWS * sizeof(char*));
    for (int i = 0; i < ROWS; i++) {
        (*board)[i] = malloc(COLS * sizeof(char));
    }
    SetupBoard(*board);
}

void teardown(char** board) {
    for (int i = 0; i < ROWS; i++) free(board[i]);
    free(board);
}

int main() {
    char** b;

    setup(&b);
    for(int i=0; i<4; i++) MakeMove(b, 1, 'A');
    assert(CheckWinner(b) == 'A');
    teardown(b);

    setup(&b);
    for(int i=1; i<=4; i++) MakeMove(b, i, 'B');
    assert(CheckWinner(b) == 'B');
    teardown(b);

    setup(&b);
    MakeMove(b, 1, 'A');
    MakeMove(b, 2, 'B'); MakeMove(b, 2, 'A');
    MakeMove(b, 3, 'B'); MakeMove(b, 3, 'B'); MakeMove(b, 3, 'A');
    MakeMove(b, 4, 'B'); MakeMove(b, 4, 'B'); MakeMove(b, 4, 'B'); MakeMove(b, 4, 'A');
    assert(CheckWinner(b) == 'A');
    teardown(b);

    printf("test_game PASSED\n");
    return 0;
}