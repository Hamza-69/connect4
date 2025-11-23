#ifndef SOLVER_H
#define SOLVER_H

// Initialize the solver (calculates bitmasks, loads book, allocates memory)
void InitSolver();

// Free solver resources
void FreeSolver();

// Get the best move for the current board state (returns 1-7)
int GetSolverMove(char** board, char current_player);

#endif