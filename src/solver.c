#include "../include/solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define WIDTH 7
#define HEIGHT 6
#define MIN_SCORE (-(WIDTH * HEIGHT) / 2 + 3)
#define MAX_SCORE ((WIDTH * HEIGHT + 1) / 2 - 3)
#define TABLE_SIZE_LOG 24 

typedef uint64_t position_t;

static uint64_t med(uint64_t min, uint64_t max) {
    return (min + max) / 2;
}

static int has_factor(uint64_t n, uint64_t min, uint64_t max) {
    if (min * min > n) return 0;
    if (min + 1 >= max) return n % min == 0;
    return has_factor(n, min, med(min, max)) || has_factor(n, med(min, max), max);
}

static uint64_t next_prime(uint64_t n) {
    return has_factor(n, 2, n) ? next_prime(n + 1) : n;
}

static inline int popcount(position_t n) {
    return __builtin_popcountll(n);
}

typedef struct {
    position_t current_position; 
    position_t mask;             
    int moves;                   
} Position;

static position_t bottom_mask_col[WIDTH];
static position_t column_mask_col[WIDTH];
static position_t bottom_mask = 0;
static position_t board_mask = 0;

void InitConsts() {
    bottom_mask = 0;
    board_mask = 0;
    for (int i = 0; i < WIDTH; i++) {
        bottom_mask_col[i] = 1ULL << (i * (HEIGHT + 1));
        column_mask_col[i] = ((1ULL << HEIGHT) - 1) << (i * (HEIGHT + 1));
        bottom_mask |= bottom_mask_col[i];
        board_mask |= column_mask_col[i];
    }
}

void Position_init(Position* p) {
    p->current_position = 0;
    p->mask = 0;
    p->moves = 0;
}

int Position_canPlay(const Position* p, int col) {
    return (p->mask & (1ULL << ((HEIGHT - 1) + col * (HEIGHT + 1)))) == 0;
}

void Position_play_move(Position* p, position_t move) {
    p->current_position ^= p->mask;
    p->mask |= move;
    p->moves++;
}

void Position_play_col(Position* p, int col) {
    Position_play_move(p, (p->mask + bottom_mask_col[col]) & column_mask_col[col]);
}

position_t Position_compute_winning_position(position_t pos, position_t mask) {
    // Vertical
    position_t r = (pos << 1) & (pos << 2) & (pos << 3);

    // Horizontal
    position_t p = (pos << (HEIGHT + 1)) & (pos << 2 * (HEIGHT + 1));
    r |= p & (pos << 3 * (HEIGHT + 1));
    r |= p & (pos >> (HEIGHT + 1));
    p = (pos >> (HEIGHT + 1)) & (pos >> 2 * (HEIGHT + 1));
    r |= p & (pos << (HEIGHT + 1));
    r |= p & (pos >> 3 * (HEIGHT + 1));

    // Diagonal 1
    p = (pos << HEIGHT) & (pos << 2 * HEIGHT);
    r |= p & (pos << 3 * HEIGHT);
    r |= p & (pos >> HEIGHT);
    p = (pos >> HEIGHT) & (pos >> 2 * HEIGHT);
    r |= p & (pos << HEIGHT);
    r |= p & (pos >> 3 * HEIGHT);

    // Diagonal 2
    p = (pos << (HEIGHT + 2)) & (pos << 2 * (HEIGHT + 2));
    r |= p & (pos << 3 * (HEIGHT + 2));
    r |= p & (pos >> (HEIGHT + 2));
    p = (pos >> (HEIGHT + 2)) & (pos >> 2 * (HEIGHT + 2));
    r |= p & (pos << (HEIGHT + 2));
    r |= p & (pos >> 3 * (HEIGHT + 2));

    return r & (board_mask ^ mask);
}

int Position_canWinNext(const Position* p) {
    return (Position_compute_winning_position(p->current_position, p->mask) & ((p->mask + bottom_mask) & board_mask)) != 0;
}

position_t Position_possibleNonLosingMoves(const Position* p) {
    position_t possible_mask = (p->mask + bottom_mask) & board_mask;
    position_t opponent_win = Position_compute_winning_position(p->current_position ^ p->mask, p->mask);
    position_t forced_moves = possible_mask & opponent_win;
    if (forced_moves) {
        if (forced_moves & (forced_moves - 1)) return 0; 
        possible_mask = forced_moves;
    }
    return possible_mask & ~(opponent_win >> 1);
}

int Position_moveScore(const Position* p, position_t move) {
    return popcount(Position_compute_winning_position(p->current_position | move, p->mask));
}

uint64_t Position_key(const Position* p) { return p->current_position + p->mask; }

void partialKey3(uint64_t *key, int col, const Position* p) {
    for (position_t pos = 1ULL << (col * (HEIGHT + 1)); pos & p->mask; pos <<= 1) {
        *key *= 3;
        if (pos & p->current_position) *key += 1;
        else *key += 2;
    }
    *key *= 3;
}

uint64_t Position_key3(const Position* p) {
    uint64_t key_forward = 0;
    for (int i = 0; i < WIDTH; i++) partialKey3(&key_forward, i, p);
    uint64_t key_reverse = 0;
    for (int i = WIDTH; i--;) partialKey3(&key_reverse, i, p);
    return key_forward < key_reverse ? key_forward / 3 : key_reverse / 3;
}

typedef struct {
    position_t move;
    int score;
} MoveEntry;

typedef struct {
    MoveEntry entries[WIDTH];
    int size;
} MoveSorter;

void MoveSorter_add(MoveSorter* ms, position_t move, int score) {
    int pos = ms->size++;
    // EXACT MATCH: Keeps list stable, sorted ascending.
    for (; pos && ms->entries[pos - 1].score > score; --pos) {
        ms->entries[pos] = ms->entries[pos - 1];
    }
    ms->entries[pos].move = move;
    ms->entries[pos].score = score;
}

position_t MoveSorter_getNext(MoveSorter* ms) {
    if (ms->size) return ms->entries[--ms->size].move;
    return 0;
}

typedef struct { uint32_t key; uint8_t val; } TTEntry; 

typedef struct {
    TTEntry* entries;
    size_t size;
} SolverTT;

typedef struct {
    void* keys; uint8_t* values; size_t size;
    int key_bytes, depth, width, height;
} OpeningBook;

void OpeningBook_load(OpeningBook* book, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        // Silently fail or warn, but don't crash. Bot will just compute.
        return; 
    }
    
    char w, h, d, kb, vb, ls;
    if (fread(&w, 1, 1, f) && fread(&h, 1, 1, f) && fread(&d, 1, 1, f) &&
        fread(&kb, 1, 1, f) && fread(&vb, 1, 1, f) && fread(&ls, 1, 1, f)) {
        
        book->size = next_prime(1ULL << ls); 
        book->width = w; book->height = h; book->depth = d; book->key_bytes = kb;
        
        book->keys = malloc(book->size * kb);
        book->values = malloc(book->size * vb);
        if (book->keys && book->values) {
            fread(book->keys, kb, book->size, f);
            fread(book->values, vb, book->size, f);
        }
    }
    fclose(f);
}

int OpeningBook_get(const OpeningBook* book, const Position* p) {
    if (!book->keys || p->moves > book->depth) return 0;
    uint64_t k = Position_key3(p);
    size_t i = k % book->size;
    uint64_t stored_key = 0;
    
    if (book->key_bytes == 1) stored_key = ((uint8_t*)book->keys)[i];
    else if (book->key_bytes == 2) stored_key = ((uint16_t*)book->keys)[i];
    else if (book->key_bytes == 4) stored_key = ((uint32_t*)book->keys)[i];
    else if (book->key_bytes == 8) stored_key = ((uint64_t*)book->keys)[i];
    
    if (stored_key == (uint64_t)((k & ((1ULL << (book->key_bytes * 8)) - 1))))
        return book->values[i];
    return 0;
}

typedef struct {
    SolverTT transTable;
    OpeningBook book;
    unsigned long long nodeCount;
    int columnOrder[WIDTH];
} Solver;

static Solver globalSolver;

void InitSolver() {
    InitConsts();
    globalSolver.transTable.size = next_prime(1 << TABLE_SIZE_LOG); 
    globalSolver.transTable.entries = (TTEntry*)calloc(globalSolver.transTable.size, sizeof(TTEntry));
    OpeningBook_load(&globalSolver.book, "7x6.book");
    
    for (int i = 0; i < WIDTH; i++) {
        globalSolver.columnOrder[i] = WIDTH / 2 + (1 - 2 * (i % 2)) * (i + 1) / 2;
    }
}

void FreeSolver() {
    if (globalSolver.transTable.entries) free(globalSolver.transTable.entries);
    if (globalSolver.book.keys) free(globalSolver.book.keys);
    if (globalSolver.book.values) free(globalSolver.book.values);
}

int Solver_negamax(Solver* s, const Position* P, int alpha, int beta) {
    s->nodeCount++;

    position_t possible = Position_possibleNonLosingMoves(P);
    if (possible == 0) return -(WIDTH * HEIGHT - P->moves) / 2;
    if (P->moves >= WIDTH * HEIGHT - 2) return 0;

    int min = -(WIDTH * HEIGHT - 2 - P->moves) / 2;
    if (alpha < min) { alpha = min; if (alpha >= beta) return alpha; }

    int max = (WIDTH * HEIGHT - 1 - P->moves) / 2;
    if (beta > max) { beta = max; if (alpha >= beta) return beta; }

    position_t key = Position_key(P);
    size_t idx = key % s->transTable.size;
    
    if (s->transTable.entries[idx].key == (uint32_t)key) {
        int val = s->transTable.entries[idx].val;
        if (val) {
            if (val > MAX_SCORE - MIN_SCORE + 1) {
                min = val + 2 * MIN_SCORE - MAX_SCORE - 2;
                if (alpha < min) { alpha = min; if (alpha >= beta) return alpha; }
            } else {
                max = val + MIN_SCORE - 1;
                if (beta > max) { beta = max; if (alpha >= beta) return beta; }
            }
        }
    }

    int book_val = OpeningBook_get(&s->book, P);
    if (book_val) return book_val + MIN_SCORE - 1;

    MoveSorter moves;
    moves.size = 0;
    
    for (int i = WIDTH - 1; i >= 0; i--) {
        int col = s->columnOrder[i];
        position_t move = possible & column_mask_col[col];
        if (move) {
            MoveSorter_add(&moves, move, Position_moveScore(P, move));
        }
    }

    while (moves.size > 0) {
        position_t next = MoveSorter_getNext(&moves);
        Position P2 = *P;
        Position_play_move(&P2, next);

        int score = -Solver_negamax(s, &P2, -beta, -alpha);
        
        if (score >= beta) {
            s->transTable.entries[idx].key = (uint32_t)key;
            s->transTable.entries[idx].val = score + MAX_SCORE - 2 * MIN_SCORE + 2;
            return score;
        }
        if (score > alpha) alpha = score;
    }
    
    s->transTable.entries[idx].key = (uint32_t)key;
    s->transTable.entries[idx].val = alpha - MIN_SCORE + 1;
    return alpha;
}

void BoardToPosition(char** board, char current_player, Position* p) {
    Position_init(p);
    for (int col = 0; col < WIDTH; col++) {
        for (int row = HEIGHT - 1; row >= 0; row--) {
            char cell = board[row][col];
            if (cell != '.') {
                int r_idx = HEIGHT - 1 - row; 
                int bit_idx = r_idx + col * (HEIGHT + 1);
                
                p->mask |= (1ULL << bit_idx);
                if (cell == current_player) {
                    p->current_position |= (1ULL << bit_idx);
                }
                p->moves++;
            }
        }
    }
}

int GetSolverMove(char** board, char current_player) {
    Position P;
    BoardToPosition(board, current_player, &P);
    for (int i = 0; i < WIDTH; i++) {
        int col = globalSolver.columnOrder[i];
        if (Position_canPlay(&P, col)) {
             if ((Position_compute_winning_position(P.current_position, P.mask) & ((P.mask + bottom_mask) & board_mask) & column_mask_col[col]) != 0) {
                 return col + 1;
            }
        }
    }

    int bestCol = -1;
    int maxScore = -999999; 
    
    for (int i = 0; i < WIDTH; i++) {
        int col = globalSolver.columnOrder[i];
        if (Position_canPlay(&P, col)) {
            Position P2 = P;
            Position_play_col(&P2, col);
            
            int score;
            
            if (Position_canWinNext(&P2)) {
                score = -(WIDTH * HEIGHT - P2.moves) / 2;
            } else {
                score = -Solver_negamax(&globalSolver, &P2, -999999, 999999);
            }
            // -------------------------

            if (score > maxScore) {
                maxScore = score;
                bestCol = col;
            }
        }
    }
    
    if (bestCol == -1) {
        for(int i=0; i<WIDTH; i++) if(Position_canPlay(&P, i)) return i+1;
    }
    
    return bestCol + 1;
}