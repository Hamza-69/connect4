# Time Complexity Analysis

## Board Operations (`src/board.c`)

| Function | Time Complexity | Description |
| :--- | :--- | :--- |
| `SetupBoard` | $O(R \times C)$ | Iterates through every cell to initialize it. |
| `PrintBoard` | $O(R \times C)$ | Iterates through every cell to print the grid. |
| `CheckMove` | $O(1)$ | Accesses the top row array index directly. |
| `MakeMove` | $O(R)$ | Scans the column from bottom to top to find the first empty spot. |
| `RemovePiece` | $O(R)$ | Scans the column from top to bottom to find the piece to remove. |
| `AnimateDrop` | $O(R^2 \times C)$ | Loops through rows ($O(R)$), calling `PrintBoard` ($O(R \times C)$) at each step. |
| `InitHistory` | $O(1)$ | Constant time initialization. |
| `RecordMove` | $O(1)$ | Array access and index increment. |
| `PerformUndo` | $O(R)$ | constant stack operations, but calls `RemovePiece` ($O(R)$). |
| `PerformRedo` | $O(R)$ | constant stack operations, but calls `MakeMove` ($O(R)$). |

## Game Logic (`src/game.c`)

| Function | Time Complexity | Description |
| :--- | :--- | :--- |
| `CheckWinner` | $O(R \times C)$ | Iterates through the board, performing constant-time checks (4 steps) for neighbors in 4 directions. |
| `RunGameLoop` | $O(N \times (R^2 \times C))$ | $N$ is moves played. Dominated by `AnimateDrop` and user I/O. |
| `GetSolverMoveThreaded` | See Solver | Wrapper for thread creation and solver invocation. |

## Solver (`src/solver.c`)

| Function | Time Complexity | Description |
| :--- | :--- | :--- |
| `Position_canWinNext` | $O(1)$ | Bitwise operations on 64-bit integers. |
| `Position_moveScore` | $O(1)$ | Bitwise operations and population count. |
| `Solver_negamax` | $O(W^D)$ | Exponential relative to depth, where $W=7$. Optimized via Alpha-Beta pruning, Transposition Tables ($O(1)$ lookup), and Move Ordering to significantly reduce effective search space. |
| `GetSolverMove` | $O(W^D)$ | Calls `Solver_negamax`. |
| `OpeningBook_get` | $O(1)$ | Hash calculation and direct array lookup. |