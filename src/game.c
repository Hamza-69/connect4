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
  printf("Choose game mode:\n1. Player vs Player\n2. Player vs Computer\n");
  printf("Enter your choice (1 or 2): ");
  fflush(stdout);
  int mode;
  while (1) {
    if (scanf("%d", &mode) != 1) {
        while (getchar() != '\n');      // clear invalid input
        printf("Invalid input! Please enter 1 or 2: ");
        fflush(stdout);
    } else if (mode != 1 && mode != 2) {
        printf("Invalid choice! Please enter 1 or 2: ");
        fflush(stdout);
    } else {
        break;
    }
  }

  if (mode == 2) {
    int difficulty;
    printf("Choose difficulty:\n1. Easy\n2. Medium\nEnter your choice (1 or 2): ");
    fflush(stdout);
    while (1) {
      if (scanf("%d", &difficulty) != 1) {
        while (getchar() != '\n');
        printf("Invalid input! Please enter 1 or 2: ");
        fflush(stdout);
      } else if (difficulty != 1 && difficulty != 2) {
        printf("Invalid choice! Please enter 1 or 2: ");
        fflush(stdout);
      } else {
        break;
      }
    }
    return difficulty+1; // 2 for easy, 3 for medium
  }
  return mode; // 1 for PvP
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

void PlayMediumBot() {
  char** board = (char**) malloc(ROWS * sizeof(char*));
  for (int i = 0; i < ROWS; i++) {
    board[i] = (char*) malloc(COLS * sizeof(char));
  }

  SetupBoard(board);

  printf("Medium Bot Complexity: O(COLS * ROWS) for win/block checks, O(COLS) for center preference\n");

  for (int i = 0; i < ROWS * COLS; i++) {
    char player = i % 2 == 0 ? 'A' : 'B';
    PrintTurn(player, board);
    int move = 0;

    if (player == 'B') {
      int found = 0;
      for (int col = 1; col <= COLS; col++) {
        if (CheckMove(col, board)) {
          MakeMove(board, col, 'B');
          if (CheckWinner(board) == 'B') {
            move = col;
            found = 1;
          }
          for (int r = 0; r < ROWS; r++) {
            if (board[r][col-1] == 'B') {
              board[r][col-1] = '.';
              break;
            }
          }
          if (found) break;
        }
      }
      if (!found) {
        for (int col = 1; col <= COLS; col++) {
          if (CheckMove(col, board)) {
            MakeMove(board, col, 'A');
            if (CheckWinner(board) == 'A') {
              move = col;
              found = 1;
            }
            for (int r = 0; r < ROWS; r++) {
              if (board[r][col-1] == 'A') {
                board[r][col-1] = '.';
                break;
              }
            }
            if (found) break;
          }
        }
      }
      if (!found) {
        int centers[] = {4, 3, 5, 2, 6, 1, 7};
        for (int j = 0; j < COLS; j++) {
          int col = centers[j];
          if (CheckMove(col, board)) {
            move = col;
            found = 1;
            break;
          }
        }
      }
      if (!found) {
        for (int col = 1; col <= COLS; col++) {
          if (CheckMove(col, board)) {
            move = col;
            break;
          }
        }
      }
      printf("Medium Bot chooses column %d\n", move);
      fflush(stdout);
    } else {
      while (1) {
        if (scanf("%d", &move) != 1) {
          while (getchar() != '\n');
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