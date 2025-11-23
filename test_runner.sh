#!/bin/bash
set -e
mkdir -p build

gcc -Wall -Wextra -std=c99 -Iinclude -o build/test_board tests/test_board.c src/board.c
gcc -Wall -Wextra -std=c99 -Iinclude -o build/test_game tests/test_game.c src/board.c src/game.c src/solver.c -pthread

./build/test_board
./build/test_game

echo "All unit tests passed."