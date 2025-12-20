#ifndef GUESS_GAME_V3_H
#define GUESS_GAME_V3_H

#include <stdbool.h>
#include <stdint.h>

#define DEFAULT_MAX_ATTEMPTS 10
#define DEFAULT_MAX_INPUT_LENGTH 10
#define DEFAULT_MIN_RANGE 1
#define DEFAULT_MAX_RANGE 10

// 错误代码定义
typedef enum {
    ERROR_NONE = 0,
    ERROR_INVALID_INPUT,
    ERROR_INPUT_TOO_LONG,
    ERROR_INPUT_OUT_OF_RANGE,
    ERROR_EMPTY_INPUT,
    ERROR_IO_FAILURE,
    ERROR_MEMORY_ALLOCATION,
    ERROR_UNKNOWN
} ErrorCode;

typedef enum {
    INPUT_VALID,
    INPUT_INVALID_NUMBER,
    INPUT_OUT_OF_RANGE,
    INPUT_TOO_LONG,
    INPUT_EMPTY,
    INPUT_IO_ERROR
} InputStatus;

typedef struct {
    int max_attempts;
    int max_input_length;
    int min_range;
    int max_range;
} GameConfigV2;

// 通用函数
void flush_output(void);
void clear_input_buffer(void);
int random_range(int min, int max);
bool is_numeric_string(const char* str);
int parse_integer(const char *str, int *result);
bool is_valid_integer_format(const char *str);
InputStatus validate_input(const char* input, const GameConfigV2* config, int* value);

int start_guessing_game_v3(void);
int start_guessing_game_v3_improved(int max_attempts, int max_input_length, int min_range, int max_range);

#endif // !GUESS_GAME_V3_H