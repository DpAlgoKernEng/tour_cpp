#include "guess_game_v2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int start_guessing_game_v2(void) {
  
    printf("=== Guess the Number Game V2 ===\n");
    printf("Guess the number between 1 and 10!\n\n");
    
    // 生成随机数
    srand((unsigned int)time(NULL));
    int secret_number = 1 + (rand() % 10);
    
    int attempts = 0;
    char input[256];
    
    while (1) {
        printf("Please input your guess: ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Failed to read input.\n");
            continue;
        }
        
        // 移除换行符
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        // 尝试解析整数
        char *endptr;
        int guess = strtol(input, &endptr, 10);
      
        // 检查是否解析成功
        if (endptr == input || *endptr != '\0') {
            printf("Invalid input. Please enter a valid number.\n\n");
            continue;
        }
        
        attempts++;
        printf("You guessed: %d\n", guess);
        
        if (guess < secret_number) {
            printf("Too small!\n");
        } else if (guess > secret_number) {
            printf("Too big!\n");
        } else {
            printf("You win! The number was %d\n", secret_number);
            printf("It took you %d attempts.\n", attempts);
            break;
        }
        
        printf("\n");
    }
    return 0;
}