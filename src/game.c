#include "../include/game.h"
#include "../include/board.h"
#include <stdlib.h>
#include <stdio.h>
#include <../include/solver.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 4444
#define BUFFER_SIZE 1024

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
  printf("Choose game mode:\n1. Player vs Player\n2. Player vs Computer\n3. Network Multiplayer (LAN)\n");
  printf("Enter your choice (1, 2 or 3): ");
  fflush(stdout);
  int mode;
  while (1) {
    if (scanf("%d", &mode) != 1) {
        while (getchar() != '\n');      // clear invalid input
        printf("Invalid input! Please enter 1, 2 or 3: ");
        fflush(stdout);
    } else if (mode < 1 || mode > 3) {
        printf("Invalid choice! Please enter 1, 2 or 3: ");
        fflush(stdout);
    } else {
        break;
    }
  }

  if (mode == 2) {
    int difficulty;
    printf("Choose difficulty:\n1. Easy\n2. Medium\n3. Hard\n");
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
  } else if (mode == 3) {
    int netMode;
    printf("Network Mode:\n1. Host (Server)\n2. Join (Client)\n");
    printf("Enter your choice (1 or 2): ");
    fflush(stdout);
    while (1) {
        if (scanf("%d", &netMode) != 1) {
            while (getchar() != '\n');
            printf("Invalid input! Please enter 1 or 2: ");
            fflush(stdout);
        } else if (netMode != 1 && netMode != 2) {
            printf("Invalid choice! Please enter 1 or 2: ");
            fflush(stdout);
        } else {
            break;
        }
    }
    return netMode == 1 ? 5 : 6; // 5=Server, 6=Client
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

// ================= NETWORK MULTIPLAYER =================

void PlayNetworkServer() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // 1. Create socket [Lecture 18]
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Optional: Allow port reuse
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 2. Bind [Lecture 18]
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. Listen [Lecture 18]
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for opponent to join on port %d...\n", PORT);

    // 4. Accept [Lecture 18]
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("Opponent connected!\n");
    sleep(1);

    char** board = (char**) malloc(ROWS * sizeof(char*));
    for (int i = 0; i < ROWS; i++) board[i] = (char*) malloc(COLS * sizeof(char));
    SetupBoard(board);

    // Server is always Player A
    for (int i = 0; i < ROWS * COLS; i++) {
        char player = (i % 2 == 0) ? 'A' : 'B';
        int move = 0;

        system("clear");
        PrintIntro();
        PrintBoard(board);

        if (player == 'A') {
            // Server's turn (You)
            printf("\nYour turn (A). Choose column (1-7): ");
            fflush(stdout);
            while (1) {
                if (scanf("%d", &move) != 1) {
                    while (getchar() != '\n');
                    printf("Invalid input! Enter 1-7: ");
                    fflush(stdout);
                } else if (move < 1 || move > COLS || !CheckMove(move, board)) {
                    printf("Invalid move! Enter 1-7: ");
                    fflush(stdout);
                } else {
                    break;
                }
            }
            
            // Send move to client [Lecture 18 write]
            sprintf(buffer, "%d", move);
            write(client_fd, buffer, strlen(buffer));

        } else {
            // Client's turn (Opponent)
            printf("\nWaiting for Opponent (B)...\n");
            fflush(stdout);
            
            // Read move from client [Lecture 18 read]
            memset(buffer, 0, BUFFER_SIZE);
            int valread = read(client_fd, buffer, BUFFER_SIZE);
            if (valread <= 0) {
                printf("Opponent disconnected.\n");
                break;
            }
            move = atoi(buffer);
        }

        MakeMove(board, move, player);

        if (CheckWinner(board) == player) {
            system("clear");
            PrintIntro();
            PrintBoard(board);
            printf("\nPlayer %c wins!\n", player);
            close(client_fd);
            close(server_fd);
            free(board);
            return;
        }
    }
    
    printf("\nIt's a draw!\n");
    close(client_fd);
    close(server_fd);
    free(board);
}

void PlayNetworkClient() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char ip_str[100];

    printf("Enter Server IP Address (press Enter for localhost/same machine): ");
    fflush(stdout);
    
    // Clear any leftover input
    while (getchar() != '\n');
    
    // Read IP address or use default
    if (fgets(ip_str, sizeof(ip_str), stdin) == NULL || ip_str[0] == '\n') {
        strcpy(ip_str, "127.0.0.1");
        printf("Using default: 127.0.0.1 (localhost - same machine)\n");
    } else {
        // Remove trailing newline
        ip_str[strcspn(ip_str, "\n")] = 0;
    }

    // 1. Create socket [Lecture 17]
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP [Lecture 17]
    if(inet_pton(AF_INET, ip_str, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    // 2. Connect [Lecture 17]
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }
    printf("Connected to server!\n");
    sleep(1);

    char** board = (char**) malloc(ROWS * sizeof(char*));
    for (int i = 0; i < ROWS; i++) board[i] = (char*) malloc(COLS * sizeof(char));
    SetupBoard(board);

    // Client is always Player B
    for (int i = 0; i < ROWS * COLS; i++) {
        char player = (i % 2 == 0) ? 'A' : 'B';
        int move = 0;

        system("clear");
        PrintIntro();
        PrintBoard(board);

        if (player == 'B') {
            // Client's turn (You)
            printf("\nYour turn (B). Choose column (1-7): ");
            fflush(stdout);
            while (1) {
                if (scanf("%d", &move) != 1) {
                    while (getchar() != '\n');
                    printf("Invalid input! Enter 1-7: ");
                    fflush(stdout);
                } else if (move < 1 || move > COLS || !CheckMove(move, board)) {
                    printf("Invalid move! Enter 1-7: ");
                    fflush(stdout);
                } else {
                    break;
                }
            }
            
            // Send move to server [Lecture 17 write]
            sprintf(buffer, "%d", move);
            write(sock, buffer, strlen(buffer));

        } else {
            // Server's turn (Opponent)
            printf("\nWaiting for Opponent (A)...\n");
            fflush(stdout);
            
            // Read move from server [Lecture 17 read]
            memset(buffer, 0, BUFFER_SIZE);
            int valread = read(sock, buffer, BUFFER_SIZE);
            if (valread <= 0) {
                printf("Server disconnected.\n");
                break;
            }
            move = atoi(buffer);
        }

        MakeMove(board, move, player);

        if (CheckWinner(board) == player) {
            system("clear");
            PrintIntro();
            PrintBoard(board);
            printf("\nPlayer %c wins!\n", player);
            close(sock);
            free(board);
            return;
        }
    }
    
    printf("\nIt's a draw!\n");
    close(sock);
    free(board);
}