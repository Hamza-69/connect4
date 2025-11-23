#!/bin/bash
set -e
make clean
make

mkdir -p tests/results
echo "Running Valgrind..."

valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --log-file=tests/results/valgrind.log \
         ./connect4 < tests/input.in

echo "Valgrind finished. Check tests/results/valgrind.log"