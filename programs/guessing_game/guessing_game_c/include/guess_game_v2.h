#ifndef GUESS_GAME_V2_H
#define GUESS_GAME_V2_H

#define MAX_ATTEMPTS 10
#define MAX_INPUT_LENGTH 10
#define MIN_RANGE 1
#define MAX_RANGE 10

#define CLEAR_BUFFER() do { \
    int ch; \
    while ((ch = getchar()) != '\n' && ch != EOF); \
} while(0)

int start_guessing_game_v2();

#endif // !GUESS_GAME_V2_H