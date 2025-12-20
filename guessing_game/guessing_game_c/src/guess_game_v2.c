#include "guess_game_v2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

// 安全的整数输入函数
int get_safe_int(int min, int max, const char *prompt) {
    char buffer[MAX_INPUT_LENGTH];
    int valid_input = 0;
    int value = 0;
    
    while (!valid_input) {
        printf("%s", prompt);
        fflush(stdout);
        
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Failed to read input.\n");
            CLEAR_BUFFER();
            continue;
        }
        
        // 检查输入是否过长
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] != '\n') {
            printf("Input too long! Please enter a number between %d and %d.\n", 
                   min, max);
            CLEAR_BUFFER();
            continue;
        }
        
        // 移除换行符
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        // 解析输入
        char *endptr;
        errno = 0;
        long temp = strtol(buffer, &endptr, 10);
        
        // 验证输入
        if (endptr == buffer) {
            printf("Invalid input. Please enter a valid number.\n");
        } else if (*endptr != '\0') {
            printf("Invalid characters in input.\n");
        } else if (errno == ERANGE || temp < INT_MIN || temp > INT_MAX) {
            printf("Number out of range.\n");
        } else if (temp < min || temp > max) {
            printf("Please enter a number between %d and %d.\n", min, max);
        } else {
            value = (int)temp;
            valid_input = 1;
        }
    }
    
    return value;
}

// 修改主函数使用安全输入
int start_guessing_game_v2() {
    printf("=== Guess the Number Game V2 ===\n");
    printf("Guess the number between 1 and 10!\n\n");
    
    srand((unsigned int)time(NULL));
    int secret_number = MIN_RANGE + (rand() % (MAX_RANGE - MIN_RANGE + 1));
    
    int attempts = 0;

    while (1) {
        if (attempts >= MAX_ATTEMPTS) {
            printf("\nGame over! You've used all %d attempts.\n", MAX_ATTEMPTS);
            printf("The secret number was: %d\n", secret_number);
            break;
        }
        
        // 使用安全输入函数
        int guess = get_safe_int(MIN_RANGE, MAX_RANGE, "Please input your guess: ");
        
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