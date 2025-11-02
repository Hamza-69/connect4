#include "../include/game.h"

int main() {
    int mode = chooseMode();
    if (mode == 1)
    Play();
    else if (mode == 2)
    PlayEasyBot();
    else if (mode == 3)
    PlayMediumBot();
    return 0;
}