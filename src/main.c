#include "../include/game.h"
#include <stdio.h>

int main() {
    while (1) {
        int mode = chooseMode();
        if (mode == -1) {
            printf("\n\033[1;36mThanks for playing. Goodbye!\033[0m\n");
            break; 
        }

        if (mode == 1) {
            Play();
        } else if (mode == 5) {
            PlayNetworkServer();
        } else if (mode == 6) {
            PlayNetworkClient();
        } else {
            char startingPlayer = chooseStartingPlayer();
            if (startingPlayer == '0') continue; // Back pressed

            if (mode == 2) PlayEasyBot(startingPlayer);
            else if (mode == 3) PlayMediumBot(startingPlayer);
            else if (mode == 4) PlayHardBot(startingPlayer);
        }
    }
    return 0;
}