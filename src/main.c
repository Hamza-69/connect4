#include "../include/game.h"

int main() {
    int mode = chooseMode();
    if (mode == 1) {
        Play();
    } else {
        char startingPlayer = chooseStartingPlayer();
        if (mode == 2)
            PlayEasyBot(startingPlayer);
        else if (mode == 3)
            PlayMediumBot(startingPlayer);
        else if (mode == 4) 
            PlayHardBot(startingPlayer);
    }
    return 0;
}