#include "../include/game.h"
#include <stdio.h>

int main() {
    int mode = chooseMode();
    if (mode == 1) {
        Play();
    } else if (mode == 5) {
        PlayNetworkServer();
    } else if (mode == 6) {
        PlayNetworkClient();
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