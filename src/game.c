#include "../include/game.h"
#include "../include/board.h"
#include <stdlib.h>
#include <stdio.h>

void PrintIntro() {
  printf("Welcome to Connect Four!\nPlayer A: A\nPlayer B: B\n");
}

void PrintTurn(char lett, char** arr) {
  system("clear"); 
  PrintIntro();
  PrintBoard(arr);
  printf("\nPlayer %c, choose a column (1-7): ", lett);
  fflush(stdout);
}

int chooseMode() {
  system("clear"); 
  printf("Choose game mode:\n1. Player vs Player\n2. Player vs Easy Computer\n");
  printf("Enter your choice (1 or 2): ");
  fflush(stdout);
  int choice;
  while (1) {
    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');      // clear invalid input
        printf("Invalid input! Please enter 1 or 2: ");
        fflush(stdout);
    } else if (choice != 1 && choice != 2) {
        printf("Invalid choice! Please enter 1 or 2: ");
        fflush(stdout);
    } else {
        break;
    }
  }
  return choice;
}

char CheckWinner(char** arr) {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
        char p = arr[i][j];
        if (p == '.') continue;

        if (j + 3 < COLS &&
            arr[i][j+1] == p && arr[i][j+2] == p && arr[i][j+3] == p)
            return p;

        if (i + 3 < ROWS &&
            arr[i+1][j] == p && arr[i+2][j] == p && arr[i+3][j] == p)
            return p;

        if (i + 3 < ROWS && j + 3 < COLS &&
            arr[i+1][j+1] == p && arr[i+2][j+2] == p && arr[i+3][j+3] == p)
            return p;

        if (i - 3 >= 0 && j + 3 < COLS &&
            arr[i-1][j+1] == p && arr[i-2][j+2] == p && arr[i-3][j+3] == p)
            return p;
    }
  }
  return '.';
}

void Play() {
  char** board = (char**) malloc(ROWS * sizeof(char**));
  for (int i = 0; i<ROWS; i++) {
    board[i] = (char*) malloc(COLS*sizeof(char));
  }

  SetupBoard(board);

  for (int i = 0; i< ROWS*COLS; i++) {
    char player = i %2 ==0 ? 'A' : 'B';
    PrintTurn(player, board);
    int move = 0;

    while (1) {
      if (scanf("%d", &move) != 1) {
          while (getchar() != '\n');      // clear invalid input
          printf("\nInvalid move!\nPlease enter a valid number form 1 to 7: ");
          fflush(stdout);
      } else if (move < 1 || move > COLS) {
          printf("\nInvalid move!\nPlease enter a valid number form 1 to 7: ");
          fflush(stdout);
      } else if (!CheckMove(move, board)) {
          printf("\nInvalid move!\nThis column is full, choose another one: ");
          fflush(stdout);
      } else {
          break;
      }
    } 
    
    MakeMove(board, move, player);
    char winner = CheckWinner(board);

    if (winner != '.' && winner == player) {
      system("clear"); 
      PrintIntro();
      PrintBoard(board);
      printf("\nPlayer %c wins!", player);
      free(board);
      return;
    }
  }
  system("clear"); 
  PrintIntro();
  PrintBoard(board);
  printf("\nIt's a draw!\n");
  free(board);
}

void PlayEasyBot() {
  char** board = (char**) malloc(ROWS * sizeof(char**));
  for (int i = 0; i<ROWS; i++) {
    board[i] = (char*) malloc(COLS*sizeof(char));
  }

  SetupBoard(board);

  for (int i = 0; i< ROWS*COLS; i++) {
    char player = i %2 ==0 ? 'A' : 'B';
    PrintTurn(player, board);
    int move = 0;

    if (player == 'B') {
      do {
        move = (rand() % 7) + 1;
      } while (!CheckMove(move, board));
      fflush(stdout);
    } else {
      while (1) {
        if (scanf("%d", &move) != 1) {
            while (getchar() != '\n');      // clear invalid input
            printf("\nInvalid move!\nPlease enter a valid number form 1 to 7: ");
          fflush(stdout);
      } else if (move < 1 || move > COLS) {
          printf("\nInvalid move!\nPlease enter a valid number form 1 to 7: ");
          fflush(stdout);
      } else if (!CheckMove(move, board)) {
          printf("\nInvalid move!\nThis column is full, choose another one: ");
          fflush(stdout);
      } else {
          break;
      }
    } 
    }
    
    MakeMove(board, move, player);
    char winner = CheckWinner(board);

    if (winner != '.' && winner == player) {
      system("clear"); 
      PrintIntro();
      PrintBoard(board);
      printf("\nPlayer %c wins!", player);
      free(board);
      return;
    }
  }
  system("clear"); 
  PrintIntro();
  PrintBoard(board);
  printf("\nIt's a draw!\n");
  free(board);
}