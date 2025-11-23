CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -Iinclude -pthread
SRC = src/main.c src/board.c src/game.c src/solver.c
OBJ = $(SRC:.c=.o)
TARGET = connect4

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test:
	chmod +x tests/test_runner.sh
	./tests/test_runner.sh

valgrind:
	chmod +x scripts/run_valgrind.sh
	./scripts/run_valgrind.sh

clean:
	rm -f $(OBJ) $(TARGET)
	rm -rf build tests/results