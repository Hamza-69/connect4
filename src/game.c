#include "../include/game.h"
#include "../include/board.h"
#include "../include/solver.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <ctype.h>

#define PORT 4444
#define BUFFER_SIZE 1024

// --- UI Colors ---
#define C_RST  "\033[0m"
#define C_RED  "\033[1;31m"
#define C_YEL  "\033[1;33m"
#define C_CYAN "\033[1;36m"
#define C_GRN  "\033[1;32m"

// --- Commands ---
#define CMD_QUIT -100
#define CMD_UNDO -101
#define CMD_REDO -102

// --- Solver Helper ---
typedef struct {
    char** board_copy;
    char player;
} SolverThreadArg;

static void free_board_copy(char** b) {
    if (!b) return;
    for (int r = 0; r < ROWS; r++) if (b[r]) free(b[r]);
    free(b);
}

static void* solver_thread_fn(void* v) {
    SolverThreadArg* a = (SolverThreadArg*)v;
    int* res = malloc(sizeof(int));
    *res = GetSolverMove(a->board_copy, a->player);
    free_board_copy(a->board_copy);
    free(a);
    return (void*)res;
}

static int GetSolverMoveThreaded(char** board_src, char player) {
    char** copy = (char**) malloc(ROWS * sizeof(char*));
    for (int i = 0; i < ROWS; i++) {
        copy[i] = (char*) malloc(COLS * sizeof(char));
        memcpy(copy[i], board_src[i], COLS * sizeof(char));
    }
    SolverThreadArg* arg = malloc(sizeof(SolverThreadArg));
    arg->board_copy = copy;
    arg->player = player;

    pthread_t tid;
    pthread_create(&tid, NULL, solver_thread_fn, arg);
    printf(C_CYAN "Bot is thinking...\n" C_RST);
    fflush(stdout);

    void* thread_res = NULL;
    pthread_join(tid, &thread_res);
    int move = thread_res ? *((int*)thread_res) : 1;
    if (thread_res) free(thread_res);
    return move;
}

// --- UI Helpers ---

void PrintTitle() {
    printf("\033[H\033[J"); 
    printf(C_CYAN);
    printf("   ______                            _  __ __\n");
    printf("  / ____/___  ____  ____  ___  _____/ /|/ // /\n");
    printf(" / /   / __ \\/ __ \\/ __ \\/ _ \\/ ___/ __/ // /_\n");
    printf("/ /___/ /_/ / / / / / / /  __/ /__/ /_/__  __/\n");
    printf("\\____/\\____/_/ /_/_/ /_/\\___/\\___/\\__/  /_/   \n");
    printf(C_RST);
    printf("\n        [ Created by Tessera ]\n\n");
}

void PrintTurn(char lett, char** arr) {
    PrintBoard(arr);
    if (lett == 'A') printf("\nPlayer " C_RED "A" C_RST "'s turn.");
    else printf("\nPlayer " C_YEL "B" C_RST "'s turn.");
    printf(" Col (1-7), 'u' undo, 'r' redo, 'q' quit: ");
    fflush(stdout);
}

int AskPlayAgain() {
    char buffer[100];
    while(1) {
        printf("\n" C_GRN "Game Over!" C_RST " Play again? (y/n): ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return 0; // EOF
        
        buffer[strcspn(buffer, "\n")] = 0; // Trim newline
        if (strlen(buffer) == 0) continue;

        char c = tolower(buffer[0]);
        if (c == 'y') return 1;
        if (c == 'n') return 0;
        printf("Invalid input.");
    }
}

int GetInputOrBack(int range_min, int range_max) {
    char buffer[100];
    while (1) {
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        buffer[strcspn(buffer, "\n")] = 0;
        
        // If empty (just enter), ignore or reprompt
        if (strlen(buffer) == 0) continue; 
        
        if (tolower(buffer[0]) == 'b') return -1;
        
        char *endptr;
        long val = strtol(buffer, &endptr, 10);
        if (endptr == buffer || *endptr != '\0' || val < range_min || val > range_max) {
            printf(C_RED "Invalid." C_RST " Enter %d-%d or 'b': ", range_min, range_max);
        } else {
            return (int)val;
        }
    }
}

int GetGameInput(int sock_fd, int is_network) {
    char buffer[100];
    while (1) {
        if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
        buffer[strcspn(buffer, "\n")] = 0;
        if (strlen(buffer) == 0) continue;

        char c = tolower(buffer[0]);
        if (c == 'q') return CMD_QUIT;
        
        if (!is_network) {
            if (c == 'u') return CMD_UNDO;
            if (c == 'r') return CMD_REDO;
        }

        char *endptr;
        long val = strtol(buffer, &endptr, 10);
        if (endptr == buffer || *endptr != '\0' || val < 1 || val > 7) {
            if (is_network) printf("Invalid! 1-7 or 'q': ");
            else printf("Invalid! 1-7, 'u', 'r', 'q': ");
        } else {
            return (int)val;
        }
    }
}

int chooseMode() {
    while (1) {
        PrintTitle();
        printf(C_YEL "1." C_RST " Player vs Player\n");
        printf(C_YEL "2." C_RST " Player vs Computer\n");
        printf(C_YEL "3." C_RST " Network Multiplayer (LAN)\n");
        printf(C_YEL "4." C_RST " Quit\n");
        printf("\nEnter choice: ");
        int mode = GetInputOrBack(1, 4);
        if (mode == -1) continue; 
        if (mode == 4) return -1;
        if (mode == 1) return 1;

        if (mode == 2) {
            printf("\nDifficulty:\n" C_YEL "1." C_RST " Easy\n" C_YEL "2." C_RST " Medium\n" C_YEL "3." C_RST " Hard\n" C_CYAN "('b' back)\n" C_RST "Choice: ");
            int diff = GetInputOrBack(1, 3);
            if (diff == -1) continue;
            return diff + 1; 
        } 
        else if (mode == 3) {
            printf("\nNetwork Mode:\n" C_YEL "1." C_RST " Host\n" C_YEL "2." C_RST " Join\n" C_CYAN "('b' back)\n" C_RST "Choice: ");
            int net = GetInputOrBack(1, 2);
            if (net == -1) continue;
            return net == 1 ? 5 : 6; 
        }
    }
}

char chooseStartingPlayer() {
    printf("\nWho starts?\n" C_YEL "1." C_RST " You\n" C_YEL "2." C_RST " Bot\n" C_CYAN "('b' back)\n" C_RST "Choice: ");
    int c = GetInputOrBack(1, 2);
    if (c == -1) return '0';
    return c == 1 ? 'A' : 'B';
}

char CheckWinner(char** arr) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            char p = arr[i][j];
            if (p == '.') continue;
            if (j + 3 < COLS && arr[i][j+1] == p && arr[i][j+2] == p && arr[i][j+3] == p) return p;
            if (i + 3 < ROWS && arr[i+1][j] == p && arr[i+2][j] == p && arr[i+3][j] == p) return p;
            if (i + 3 < ROWS && j + 3 < COLS && arr[i+1][j+1] == p && arr[i+2][j+2] == p && arr[i+3][j+3] == p) return p;
            if (i - 3 >= 0 && j + 3 < COLS && arr[i-1][j+1] == p && arr[i-2][j+2] == p && arr[i-3][j+3] == p) return p;
        }
    }
    return '.';
}

// --- Game Logic Internal ---
void RunGameLoop(int mode, char startingPlayer, int sock_fd, int is_server) {
    char** board = (char**) malloc(ROWS * sizeof(char*));
    for (int i = 0; i<ROWS; i++) board[i] = (char*) malloc(COLS*sizeof(char));
    char buffer[BUFFER_SIZE];
    GameHistory history;

    do {
        SetupBoard(board);
        InitHistory(&history);
        int gameOver = 0;
        int userQuit = 0; // Flag to track if user pressed 'q'
        if (mode == 4) InitSolver();

        int currentTurnIndex = 0; 
        
        while (!gameOver) {
            char player = ((currentTurnIndex % 2) == 0) ? startingPlayer : (startingPlayer == 'A' ? 'B' : 'A');
            
            // Check Draw
            int full = 1;
            for(int c=0; c<COLS; c++) if(CheckMove(c+1, board)) full=0;
            if(full) {
                PrintBoard(board); printf(C_GRN "\nIt's a Draw!\n" C_RST); break;
            }

            int move = 0;
            int is_bot = (mode >= 2 && mode <= 4 && player == 'B');
            int is_net = ((mode == 5 && player == 'B') || (mode == 6 && player == 'A'));

            if (is_bot) {
                PrintBoard(board);
                if (mode == 2) { // Easy
                    usleep(400000);
                    do { move = (rand() % 7) + 1; } while (!CheckMove(move, board));
                } else if (mode == 3) { // Medium
                    usleep(400000);
                    int found=0;
                    for(int c=1; c<=7; c++) if(CheckMove(c,board)){ MakeMove(board,c,'B'); if(CheckWinner(board)=='B'){move=c; found=1;} RemovePiece(board,c); if(found)break; } 
                    if(!found) for(int c=1; c<=7; c++) if(CheckMove(c,board)){ MakeMove(board,c,'A'); if(CheckWinner(board)=='A'){move=c; found=1;} RemovePiece(board,c); if(found)break; }
                    if(!found) do { move = (rand() % 7) + 1; } while (!CheckMove(move, board));
                } else { // Hard
                    move = GetSolverMoveThreaded(board, 'B');
                }
                
                RecordMove(&history, move);
                AnimateDrop(board, move, player);
                currentTurnIndex++;
            } 
            else if (is_net) {
                PrintBoard(board);
                printf(C_CYAN "\nWaiting for Opponent...\n" C_RST);
                memset(buffer, 0, BUFFER_SIZE);
                if (read(sock_fd, buffer, BUFFER_SIZE) <= 0) {
                    printf(C_RED "\nConnection lost.\n" C_RST); gameOver = 1; break;
                }
                move = atoi(buffer);
                RecordMove(&history, move);
                AnimateDrop(board, move, player);
                currentTurnIndex++;
            } 
            else {
                // Local Human
                PrintTurn(player, board);
                int input = GetGameInput(sock_fd, (mode==5 || mode==6));
                
                if (input == CMD_QUIT) {
                    gameOver = 1;
                    userQuit = 1; 
                    if(mode == 5 || mode == 6) close(sock_fd);
                    break;
                }
                else if (input == CMD_UNDO) {
                    if (mode >= 2 && mode <= 4) {
                        if (history.top >= 2) {
                            PerformUndo(&history, board);
                            PerformUndo(&history, board);
                            currentTurnIndex -= 2;
                            printf(C_YEL "Undone last round.\n" C_RST);
                        } else {
                            printf(C_RED "Cannot undo further.\n" C_RST); sleep(1);
                        }
                    } else {
                        if (history.top > 0) {
                            PerformUndo(&history, board);
                            currentTurnIndex--;
                        } else {
                            printf(C_RED "Nothing to undo.\n" C_RST); sleep(1);
                        }
                    }
                    continue;
                }
                else if (input == CMD_REDO) {
                    if (mode >= 2 && mode <= 4) {
                         if (history.redoTop > 0) {
                             int m = PerformRedo(&history, board, player);
                             if (m != -1) currentTurnIndex++;
                         } else {
                             printf(C_RED "Nothing to redo.\n" C_RST); sleep(1);
                         }
                    } else {
                         if (history.redoTop > 0) {
                            PerformRedo(&history, board, player);
                            currentTurnIndex++;
                        } else {
                            printf(C_RED "Nothing to redo.\n" C_RST); sleep(1);
                        }
                    }
                    continue;
                }
                else {
                    if (!CheckMove(input, board)) {
                         printf(C_RED "Column full!\n" C_RST); sleep(1); continue;
                    }
                    move = input;
                    RecordMove(&history, move);
                    if (mode == 5 || mode == 6) {
                        sprintf(buffer, "%d", move);
                        write(sock_fd, buffer, strlen(buffer));
                    }
                    AnimateDrop(board, move, player);
                    currentTurnIndex++;
                }
            }

            char winner = CheckWinner(board);
            if (winner != '.') {
                PrintBoard(board);
                if (winner == 'A') printf("\n" C_RED "Player A Wins!\n" C_RST);
                else printf("\n" C_YEL "Player B Wins!\n" C_RST);
                gameOver = 1;
            }
        }

        if (mode == 4) FreeSolver();
        
        // If user typed 'q', break loop immediately to go to menu
        if (userQuit) break;

        if (gameOver && AskPlayAgain() == 0) break;

    } while (1);

    for(int i=0;i<ROWS;i++) free(board[i]);
    free(board);
}

void Play() { RunGameLoop(1, 'A', 0, 0); }
void PlayEasyBot(char s) { RunGameLoop(2, s, 0, 0); }
void PlayMediumBot(char s) { RunGameLoop(3, s, 0, 0); }
void PlayHardBot(char s) { RunGameLoop(4, s, 0, 0); }

void PlayNetworkServer() {
    int s_fd, c_fd; struct sockaddr_in addr; int opt=1; socklen_t len=sizeof(addr);
    if ((s_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) return;
    setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(PORT);
    if (bind(s_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) return;
    listen(s_fd, 1);
    
    PrintTitle();
    printf(C_CYAN "Waiting for player on port %d...\n" C_RST, PORT);
    printf(C_YEL "Press 'b' + Enter to cancel.\n" C_RST);

    fd_set fds;
    while(1) {
        FD_ZERO(&fds); FD_SET(s_fd, &fds); FD_SET(STDIN_FILENO, &fds);
        select(s_fd+1, &fds, NULL, NULL, NULL);
        if(FD_ISSET(STDIN_FILENO, &fds)) { 
            char b[10]; fgets(b,10,stdin); 
            if(tolower(b[0])=='b') { close(s_fd); return; } 
        }
        if(FD_ISSET(s_fd, &fds)) {
             if((c_fd = accept(s_fd, (struct sockaddr *)&addr, &len))>=0) break;
        }
    }
    printf(C_GRN "Connected!\n" C_RST); sleep(1);
    RunGameLoop(5, 'A', c_fd, 1);
    close(c_fd); close(s_fd);
}

void PlayNetworkClient() {
    int sock; struct sockaddr_in serv; char ip[50];
    PrintTitle();
    printf("Enter IP (empty=localhost)\n" C_CYAN "('b' back)\n" C_RST "IP: ");
    
    if (!fgets(ip, 50, stdin)) return;
    ip[strcspn(ip, "\n")] = 0;
    if(tolower(ip[0])=='b') return;
    if(strlen(ip)==0) strcpy(ip, "127.0.0.1");
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return;
    serv.sin_family = AF_INET; serv.sin_port = htons(PORT);
    inet_pton(AF_INET, ip, &serv.sin_addr);
    
    printf("Connecting..."); fflush(stdout);
    if (connect(sock, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        printf(C_RED " Failed.\n" C_RST); sleep(1); return;
    }
    printf(C_GRN " Connected!\n" C_RST); sleep(1);
    RunGameLoop(6, 'A', sock, 0);
    close(sock);
}