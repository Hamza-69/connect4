#ifndef GAME_H
#define GAME_H

#define ROWS 6
#define COLS 7

void PrintIntro();
void PrintTurn(char lett, char** arr);
char CheckWinner(char** arr);
void Play();
int chooseMode();
void PlayEasyBot();
void PlayMediumBot()

#endif
