#include "../include/game.h"
#include "../include/board.h"
#include <stdlib.h>
#include <stdio.h>
#include <../include/solver.h>
#include <pthread.h>
#include <string.h>

// Threaded solver helper declarations and implementation (file scope).
typedef struct {
  char** board_copy;
  char player;
} SolverThreadArg;

static void free_board_copy(char** b) {
  if (!b) return;
  for (int r = 0; r < ROWS; r++) {
    if (b[r]) free(b[r]);
  }
  free(b);
}

static void* solver_thread_fn(void* v) {
  SolverThreadArg* a = (SolverThreadArg*)v;
  int* res = malloc(sizeof(int));
  if (!res) {
    free_board_copy(a->board_copy);
    free(a);
    return NULL;
  }
  *res = GetSolverMove(a->board_copy, a->player);
  free_board_copy(a->board_copy);
  free(a);
  return (void*)res;
}

static int GetSolverMoveThreaded(char** board_src, char player) {
  char** copy = (char**) malloc(ROWS * sizeof(char*));
  if (!copy) return GetSolverMove(board_src, player);
  for (int i = 0; i < ROWS; i++) {
    copy[i] = (char*) malloc(COLS * sizeof(char));
    if (!copy[i]) {
      for (int j = 0; j < i; j++) free(copy[j]);
      free(copy);
      return GetSolverMove(board_src, player);
    }
    memcpy(copy[i], board_src[i], COLS * sizeof(char));
  }

  SolverThreadArg* arg = malloc(sizeof(SolverThreadArg));
  if (!arg) {
    free_board_copy(copy);
    return GetSolverMove(board_src, player);
  }
  arg->board_copy = copy;
  arg->player = player;

  pthread_t tid;
  if (pthread_create(&tid, NULL, solver_thread_fn, arg) != 0) {
    free_board_copy(copy);
    free(arg);
    return GetSolverMove(board_src, player);
  }

  printf("Bot is thinking...\n");
  fflush(stdout);

  void* thread_res = NULL;
  if (pthread_join(tid, &thread_res) != 0) {
    return GetSolverMove(board_src, player);
  }

  int move = thread_res ? *((int*)thread_res) : GetSolverMove(board_src, player);
  if (thread_res) free(thread_res);
  return move;
}

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
    printf("Choose difficulty:\n1. Easy\n2. Medium\n3. Hard\n"); // <--- UPDATED
    printf("Enter your choice (1, 2, or 3): ");
    fflush(stdout);
    while (1) {
      if (scanf("%d", &difficulty) != 1) {
        while (getchar() != '\n');
        printf("Invalid input! Please enter 1, 2 or 3: ");
        fflush(stdout);
      } else if (difficulty < 1 || difficulty > 3) {
        printf("Invalid choice! Please enter 1, 2 or 3: ");
        fflush(stdout);
      } else {
        break;
      }
    }
    return difficulty + 1; // 2=Easy, 3=Medium, 4=Hard
  }
  return mode;
}

char chooseStartingPlayer() {
  printf("\nWho should start first?\n1. Player (You)\n2. Bot\n");
  printf("Enter your choice (1 or 2): ");
  fflush(stdout);
  int choice;
  while (1) {
    if (scanf("%d", &choice) != 1) {
      while (getchar() != '\n');
      printf("Invalid input! Please enter 1 or 2: ");
      fflush(stdout);
    } else if (choice != 1 && choice != 2) {
      printf("Invalid choice! Please enter 1 or 2: ");
      fflush(stdout);
    } else {
      break;
    }
  }
  return choice == 1 ? 'A' : 'B';
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

void PlayEasyBot(char startingPlayer) {
  char** board = (char**) malloc(ROWS * sizeof(char**));
  for (int i = 0; i<ROWS; i++) {
    board[i] = (char*) malloc(COLS*sizeof(char));
  }

  SetupBoard(board);

  for (int i = 0; i< ROWS*COLS; i++) {
    char player = (i % 2 == 0) ? startingPlayer : (startingPlayer == 'A' ? 'B' : 'A');
    PrintTurn(player, board);
    int move = 0;

    if (player == 'B') {
      // First, check if opponent (player 'A') can win in next move and block it
      int found_threat = 0;
      for (int col = 1; col <= COLS; col++) {
        if (CheckMove(col, board)) {
          MakeMove(board, col, 'A');
          if (CheckWinner(board) == 'A') {
            move = col;
            found_threat = 1;
          }
          // Undo the move
          for (int r = 0; r < ROWS; r++) {
            if (board[r][col-1] == 'A') {
              board[r][col-1] = '.';
              break;
            }
          }
          if (found_threat) break;
        }
      }
      
      if (!found_threat) {
        // No immediate threat, make random move
        do {
          move = (rand() % 7) + 1;
        } while (!CheckMove(move, board));
      }
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

void PlayMediumBot(char startingPlayer) {
  char** board = (char**) malloc(ROWS * sizeof(char*));
  for (int i = 0; i < ROWS; i++) {
    board[i] = (char*) malloc(COLS * sizeof(char));
  }

  SetupBoard(board);

  printf("Medium Bot Complexity: O(COLS * ROWS) for win/block checks, O(COLS) for center preference\n");

  for (int i = 0; i < ROWS * COLS; i++) {
    char player = (i % 2 == 0) ? startingPlayer : (startingPlayer == 'A' ? 'B' : 'A');
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

void PlayHardBot(char startingPlayer) {
  char** board = (char**) malloc(ROWS * sizeof(char*));
  for (int i = 0; i < ROWS; i++) {
    board[i] = (char*) malloc(COLS * sizeof(char));
  }

  SetupBoard(board);
  InitSolver();

  

  for (int i = 0; i < ROWS * COLS; i++) {
    char player = (i % 2 == 0) ? startingPlayer : (startingPlayer == 'A' ? 'B' : 'A');
    PrintTurn(player, board);
    int move = 0;

    if (player == 'B') {
      move = GetSolverMoveThreaded(board, 'B');
      printf("Bot (Hard) chooses column %d\n", move);
      fflush(stdout);
    } else {
      while (1) {
        if (scanf("%d", &move) != 1) {
            while (getchar() != '\n');      
            printf("\nInvalid move! 1-7: ");
            fflush(stdout);
        } else if (move < 1 || move > COLS) {
            printf("\nInvalid move! 1-7: ");
            fflush(stdout);
        } else if (!CheckMove(move, board)) {
            printf("\nColumn full! Choose another: ");
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
      FreeSolver();
      free(board);
      return;
    }
  }
  system("clear"); 
  PrintIntro();
  PrintBoard(board);
  printf("\nIt's a draw!\n");
  FreeSolver();
  free(board);
}