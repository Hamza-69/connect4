# Connect 4 Game

A classic Connect 4 game implementation in C, where two players take turns dropping pieces into a 6x7 grid to get four in a row.

## Project Structure

```
connect4/
├── include/
│   ├── board.h      # Board-related function declarations
│   └── game.h       # Game logic function declarations
├── src/
│   ├── main.c       # Entry point
│   ├── board.c      # Board setup and display functions
│   └── game.c       # Game logic and win detection
├── Makefile         # Build configuration
└── README.md        # This file
```

## Building and Running

### Prerequisites
- GCC compiler
- Make utility

### Build Instructions

1. **Compile the project:**
   ```bash
   make
   ```

2. **Run the game:**
   ```bash
   ./connect4
   ```

3. **Clean build artifacts:**
   ```bash
   make clean
   ```