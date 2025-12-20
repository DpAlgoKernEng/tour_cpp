#include "guess_game_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int start_guessing_game_v1() {

    printf("=== Guess the Number Game V1 ===\n");
    printf("Guess the number between 1 and 10!\n\n");  

    int user_guess = 0;

    // 生成随机数
    srand((unsigned int)time(NULL));
    int secret_number = 1 + (rand() % 10);

    while (user_guess != secret_number) {
        printf("Enter your guess: ");
        scanf("%d", &user_guess);

        if (user_guess < secret_number) {
            printf("Too low! Try again.\n");
        } else if (user_guess > secret_number) {
            printf("Too high! Try again.\n");
        } else {
            printf("Congratulations! You've guessed the number!\n");
        }
    }
    return 0;
}