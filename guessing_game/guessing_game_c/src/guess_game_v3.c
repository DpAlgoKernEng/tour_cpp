#include "guess_game_v3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

void flush_output(void) { fflush(stdout); }

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

bool is_numeric_string(const char* str) {
    if (str == NULL || *str == '\0') {
        return false;
    }
    
    // 跳过前导空格
    while (isspace((unsigned char)*str)) { str++; }
    
    // 检查符号
    if (*str == '+' || *str == '-') { str++; }
    
    bool has_digit = false;
    while (*str != '\0') {
        if (!isdigit((unsigned char)*str)) {
            // 允许尾随空格
            while (isspace((unsigned char)*str)) {
                str++;
            }
            if (*str == '\0') {
                break;
            }
            return false;
        }
        has_digit = true;
        str++;
    }
    
    return has_digit;
}

int parse_integer(const char *str, int *result) {
    char *endptr;
    long val;
  
    if (str == NULL) return 0;
  
    // 清除之前的errno
    errno = 0;
  
    // 解析为long
    val = strtol(str, &endptr, 10);
  
    // 检查是否完全没有转换
    if (endptr == str) { return 0; } // 无效输入
  
    // 跳过可能的空白字符
    while (isspace((unsigned char)*endptr)) { endptr++; }
  
    // 检查是否完全消耗了字符串
    if (*endptr != '\0') { return 0; } // 后面有非法字符
  
    // 检查溢出
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) { return -1; } // 溢出
  
    *result = (int)val;
    return 1;  // 成功
}

bool is_valid_integer_format(const char *str) {
    if (str == NULL) return false;
  
    // 跳过前导空白
    while (isspace((unsigned char)*str)) str++;
  
    // 检查符号
    if (*str == '+' || *str == '-') str++;
  
    // 必须至少有一位数字
    if (!isdigit((unsigned char)*str)) return false;
  
    // 检查所有字符都是数字
    while (isdigit((unsigned char)*str)) str++;
  
    // 跳过尾随空白
    while (isspace((unsigned char)*str)) str++;
  
    // 必须到达字符串结尾
    return *str == '\0';
}

int random_range(int min, int max) {
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
    
    return min + (rand() % (max - min + 1));
}

InputStatus validate_input(const char* input, const GameConfigV2* config, int* value) {
    if (input == NULL || *input == '\0') {
        return INPUT_EMPTY;
    }
    
    // 检查长度
    size_t len = strlen(input);
    if (len > (size_t)config->max_input_length) {
        return INPUT_TOO_LONG;
    }
    
    // 检查是否为数字
    if (!is_numeric_string(input)) {
        return INPUT_INVALID_NUMBER;
    }
    
    *value = atoi(input);
    
    // 检查范围
    if (*value < config->min_range || *value > config->max_range) {
        return INPUT_OUT_OF_RANGE;
    }
    
    return INPUT_VALID;
}

int start_guessing_game_v3(void) {
    return start_guessing_game_v3_improved(DEFAULT_MAX_ATTEMPTS, DEFAULT_MAX_INPUT_LENGTH, 
                          DEFAULT_MIN_RANGE, DEFAULT_MAX_RANGE);
}

int start_guessing_game_v3_improved(int max_attempts, int max_input_length, 
                           int min_range, int max_range) {
    printf("=== Guess the Number Game V2 ===\n");
    printf("Range: %d-%d | Max attempts: %d\n\n", min_range, max_range, max_attempts);
    
    // 配置
    GameConfigV2 config = {
        .max_attempts = max_attempts,
        .max_input_length = max_input_length,
        .min_range = min_range,
        .max_range = max_range
    };
    
    // 生成随机数
    int secret_number = random_range(min_range, max_range);
    int attempts = 0;
    char input[256];
    
    while (1) {
        // 检查是否超过最大尝试次数
        if (attempts >= config.max_attempts) {
            printf("\nGame over! You've used all %d attempts.\n", config.max_attempts);
            printf("The secret number was: %d\n", secret_number);
            break;
        }
        
        printf("Attempt %d/%d\n", attempts + 1, config.max_attempts);
        printf("Please input your guess (%d-%d): ", config.min_range, config.max_range);
        flush_output();
        
        // 读取输入
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Failed to read input.\n");
            continue;
        }
        
        // 移除换行符
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        // 验证输入
        int guess;
        InputStatus status = validate_input(input, &config, &guess);
        
        switch (status) {
            case INPUT_TOO_LONG:
                printf("Input too long! Maximum %d characters.\n", config.max_input_length);
                continue;
            case INPUT_INVALID_NUMBER:
                printf("Please enter a valid number.\n");
                continue;
            case INPUT_OUT_OF_RANGE:
                printf("Please enter a number between %d and %d.\n", 
                      config.min_range, config.max_range);
                continue;
            case INPUT_EMPTY:
                printf("Please enter a number.\n");
                continue;
            case INPUT_IO_ERROR:
                printf("I/O error occurred.\n");
                continue;
            case INPUT_VALID:
                // 继续处理
                break;
        }
        
        attempts++;
        printf("You guessed: %d\n", guess);
        
        // 比较猜测和秘密数字
        if (guess < secret_number) {
            printf("Too small!\n");
        } else if (guess > secret_number) {
            printf("Too big!\n");
        } else {
            printf("You win in %d attempts!\n", attempts);
            printf("The secret number was: %d\n", secret_number);
            break;
        }
        
        printf("\n");
    }

    return 0;
}