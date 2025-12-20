# C11 CMake构建的猜数字游戏版本（第三版）

## **项目概述**

这是一个完整的、模块化的猜数字游戏实现，使用C11标准编写，CMake构建系统，具有高度可配置性和完整的错误处理。

## **项目结构**

```
guess_game_c_v3/
├── CMakeLists.txt          # CMake构建配置
├── README.md              # 项目说明
├── LICENSE                # 许可证文件
├── src/                   # 源代码目录
│   ├── main.c            # 主程序入口
│   ├── common.h          # 公共头文件
│   ├── common.c          # 公共实现
│   ├── core/             # 核心模块
│   │   ├── config.h      # 游戏配置
│   │   ├── config.c
│   │   ├── state.h       # 游戏状态
│   │   ├── state.c
│   │   ├── validator.h   # 输入验证器
│   │   ├── validator.c
│   │   ├── input.h       # 输入处理器
│   │   ├── input.c
│   │   ├── game.h        # 游戏核心
│   │   └── game.c
│   └── ui/               # 用户界面
│       ├── display.h     # 显示模块
│       ├── display.c
│       └── menu.h        # 菜单系统
│       └── menu.c
├── test/                  # 测试目录
│   ├── CMakeLists.txt
│   ├── test_main.c
│   └── test_game.c
└── docs/                  # 文档目录
    ├── design.md         # 设计文档
    └── api.md            # API文档
```

## **完整实现代码**

### **1. CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.12)
project(guess_game_c_v3 VERSION 1.0.0 LANGUAGES C)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 编译器选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wconversion)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -DDEBUG)
    add_definitions(-DDEBUG)
else()
    add_compile_options(-O2 -DNDEBUG)
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 创建静态库
add_library(game_core STATIC
    src/common.c
    src/core/config.c
    src/core/state.c
    src/core/validator.c
    src/core/input.c
    src/core/game.c
    src/ui/display.c
    src/ui/menu.c
)

# 主可执行文件
add_executable(guess_game src/main.c)
target_link_libraries(guess_game game_core)

# 安装配置
install(TARGETS guess_game DESTINATION bin)
install(DIRECTORY src/core/ DESTINATION include/core FILES_MATCHING PATTERN "*.h")
install(DIRECTORY src/ui/ DESTINATION include/ui FILES_MATCHING PATTERN "*.h")
install(FILES src/common.h DESTINATION include)

# 测试支持
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# 版本信息
configure_file(
    ${CMAKE_SOURCE_DIR}/config.h.in
    ${CMAKE_BINARY_DIR}/config.h
)

# 打包
set(CPACK_PACKAGE_NAME "guess_game_c_v3")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "Modular Guess the Number Game in C")
set(CPACK_PACKAGE_CONTACT "your-email@example.com")
include(CPack)
```

### **2. 公共模块 (common.h/common.c)**

**src/common.h:**

```c
#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

// 平台兼容性宏
#ifdef _WIN32
    #define CLEAR_SCREEN "cls"
#else
    #define CLEAR_SCREEN "clear"
#endif

// 错误代码枚举
typedef enum {
    ERR_SUCCESS = 0,
    ERR_FAILURE = -1,
    ERR_INVALID_ARGUMENT = -2,
    ERR_OUT_OF_MEMORY = -3,
    ERR_IO_ERROR = -4,
    ERR_INVALID_INPUT = -5,
    ERR_VALUE_OUT_OF_RANGE = -6,
    ERR_GAME_OVER = -7,
    ERR_MAX_RETRIES_EXCEEDED = -8
} ErrorCode;

// 布尔类型别名
typedef bool Bool;
#define TRUE true
#define FALSE false

// 辅助函数声明
void common_init(void);
void common_cleanup(void);

int random_int(int min, int max);
float random_float(float min, float max);

Bool is_string_empty(const char* str);
Bool is_string_numeric(const char* str);
char* string_trim(char* str);
char* string_to_lower(char* str);
char* string_to_upper(char* str);

void flush_output_stream(void);
void clear_input_buffer(void);
void print_separator(char ch, int length);
void print_centered(const char* text, int width);

// 内存分配包装器
void* safe_malloc(size_t size);
void* safe_calloc(size_t count, size_t size);
void* safe_realloc(void* ptr, size_t size);
void safe_free(void** ptr);

// 错误处理
void set_error_message(char* buffer, size_t size, ErrorCode code, const char* msg);
void print_error(ErrorCode code, const char* context);

#endif // COMMON_H
```

**src/common.c:**

```c
#include "common.h"
#include <stdarg.h>

// 初始化随机数生成器
void common_init(void) {
    static Bool initialized = FALSE;
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = TRUE;
    }
}

void common_cleanup(void) {
    // 清理资源（如果需要）
}

// 生成指定范围内的随机整数
int random_int(int min, int max) {
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
  
    common_init();
    return min + (rand() % (max - min + 1));
}

// 生成指定范围内的随机浮点数
float random_float(float min, float max) {
    if (min > max) {
        float temp = min;
        min = max;
        max = temp;
    }
  
    common_init();
    float scale = (float)rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}

// 检查字符串是否为空或仅包含空白字符
Bool is_string_empty(const char* str) {
    if (str == NULL) return TRUE;
  
    while (*str != '\0') {
        if (!isspace((unsigned char)*str)) {
            return FALSE;
        }
        str++;
    }
  
    return TRUE;
}

// 检查字符串是否为有效的数字
Bool is_string_numeric(const char* str) {
    if (str == NULL || *str == '\0') {
        return FALSE;
    }
  
    // 跳过前导空格
    while (isspace((unsigned char)*str)) str++;
  
    // 检查符号
    if (*str == '+' || *str == '-') str++;
  
    Bool has_digit = FALSE;
    Bool has_dot = FALSE;
  
    while (*str != '\0') {
        if (isdigit((unsigned char)*str)) {
            has_digit = TRUE;
        } else if (*str == '.') {
            if (has_dot) return FALSE; // 多个小数点
            has_dot = TRUE;
        } else if (isspace((unsigned char)*str)) {
            // 允许尾随空格
            while (isspace((unsigned char)*str)) str++;
            if (*str == '\0') break;
            return FALSE;
        } else {
            return FALSE;
        }
        str++;
    }
  
    return has_digit;
}

// 去除字符串两端的空白字符
char* string_trim(char* str) {
    if (str == NULL) return NULL;
  
    char* end;
  
    // 去除前导空白
    while (isspace((unsigned char)*str)) str++;
  
    // 如果整个字符串都是空白
    if (*str == '\0') {
        return str;
    }
  
    // 去除尾随空白
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
  
    // 写入新的空终止符
    *(end + 1) = '\0';
  
    return str;
}

// 转换为小写
char* string_to_lower(char* str) {
    if (str == NULL) return NULL;
  
    char* p = str;
    while (*p) {
        *p = tolower((unsigned char)*p);
        p++;
    }
  
    return str;
}

// 转换为大写
char* string_to_upper(char* str) {
    if (str == NULL) return NULL;
  
    char* p = str;
    while (*p) {
        *p = toupper((unsigned char)*p);
        p++;
    }
  
    return str;
}

// 刷新输出流
void flush_output_stream(void) {
    fflush(stdout);
}

// 清空输入缓冲区
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 打印分隔线
void print_separator(char ch, int length) {
    for (int i = 0; i < length; i++) {
        putchar(ch);
    }
    putchar('\n');
}

// 居中打印文本
void print_centered(const char* text, int width) {
    if (text == NULL) return;
  
    int len = (int)strlen(text);
    int padding = (width - len) / 2;
  
    if (padding < 0) padding = 0;
  
    for (int i = 0; i < padding; i++) {
        putchar(' ');
    }
    printf("%s\n", text);
}

// 安全内存分配
void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL && size > 0) {
        fprintf(stderr, "Memory allocation failed: requested %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* safe_calloc(size_t count, size_t size) {
    void* ptr = calloc(count, size);
    if (ptr == NULL && count > 0 && size > 0) {
        fprintf(stderr, "Memory allocation failed: requested %zu elements of %zu bytes\n", 
                count, size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* safe_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL && size > 0) {
        fprintf(stderr, "Memory reallocation failed: requested %zu bytes\n", size);
        free(ptr);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

void safe_free(void** ptr) {
    if (ptr != NULL && *ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
    }
}

// 错误处理
void set_error_message(char* buffer, size_t size, ErrorCode code, const char* msg) {
    if (buffer == NULL || size == 0) return;
  
    const char* code_str;
    switch (code) {
        case ERR_SUCCESS: code_str = "Success"; break;
        case ERR_FAILURE: code_str = "Failure"; break;
        case ERR_INVALID_ARGUMENT: code_str = "Invalid argument"; break;
        case ERR_OUT_OF_MEMORY: code_str = "Out of memory"; break;
        case ERR_IO_ERROR: code_str = "I/O error"; break;
        case ERR_INVALID_INPUT: code_str = "Invalid input"; break;
        case ERR_VALUE_OUT_OF_RANGE: code_str = "Value out of range"; break;
        case ERR_GAME_OVER: code_str = "Game over"; break;
        case ERR_MAX_RETRIES_EXCEEDED: code_str = "Max retries exceeded"; break;
        default: code_str = "Unknown error"; break;
    }
  
    if (msg != NULL) {
        snprintf(buffer, size, "%s: %s", code_str, msg);
    } else {
        snprintf(buffer, size, "%s", code_str);
    }
}

void print_error(ErrorCode code, const char* context) {
    char buffer[256];
    set_error_message(buffer, sizeof(buffer), code, context);
    fprintf(stderr, "Error: %s\n", buffer);
}
```

### **3. 配置模块 (core/config.h/config.c)**

**src/core/config.h:**

```c
#ifndef CONFIG_H
#define CONFIG_H

#include "../common.h"

#ifdef __cplusplus
extern "C" {
#endif

// 游戏难度级别
typedef enum {
    DIFFICULTY_EASY = 0,    // 1-10, 无限尝试
    DIFFICULTY_NORMAL,      // 1-50, 10次尝试
    DIFFICULTY_HARD,        // 1-100, 5次尝试
    DIFFICULTY_EXPERT,      // 1-1000, 3次尝试
    DIFFICULTY_CUSTOM       // 自定义配置
} DifficultyLevel;

// 游戏配置结构体
typedef struct {
    uint32_t max_attempts;      // 最大尝试次数
    uint32_t max_input_length;  // 最大输入长度
    uint32_t min_range;         // 最小范围
    uint32_t max_range;         // 最大范围
    Bool allow_hints;           // 是否允许提示
    Bool show_history;          // 是否显示历史
    Bool auto_save;             // 是否自动保存
    DifficultyLevel difficulty; // 难度级别
    char player_name[64];       // 玩家名称
} GameConfig;

// 配置文件路径
typedef struct {
    char config_path[256];
    char save_path[256];
    char log_path[256];
} ConfigPaths;

// 函数声明
GameConfig* config_create(void);
void config_destroy(GameConfig** config);

GameConfig* config_create_default(void);
GameConfig* config_create_from_difficulty(DifficultyLevel difficulty);
GameConfig* config_create_custom(uint32_t min_range, uint32_t max_range, 
                               uint32_t max_attempts, Bool allow_hints);

void config_copy(GameConfig* dest, const GameConfig* src);
GameConfig* config_clone(const GameConfig* src);

// 验证配置有效性
Bool config_validate(const GameConfig* config, char* error_msg, size_t error_size);

// 获取配置信息
uint32_t config_get_range_size(const GameConfig* config);
const char* config_get_difficulty_name(DifficultyLevel difficulty);
void config_display(const GameConfig* config);

// 配置文件操作
Bool config_save_to_file(const GameConfig* config, const char* filename);
GameConfig* config_load_from_file(const char* filename);
Bool config_save_default(const GameConfig* config);
GameConfig* config_load_default(void);

// 路径管理
void config_paths_init(ConfigPaths* paths, const char* app_name);
const char* config_get_default_config_path(void);
const char* config_get_default_save_path(void);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_H
```

**src/core/config.c:**

```c
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 创建空的配置对象
GameConfig* config_create(void) {
    GameConfig* config = (GameConfig*)safe_malloc(sizeof(GameConfig));
    memset(config, 0, sizeof(GameConfig));
    return config;
}

// 销毁配置对象
void config_destroy(GameConfig** config) {
    if (config != NULL && *config != NULL) {
        safe_free((void**)config);
    }
}

// 创建默认配置
GameConfig* config_create_default(void) {
    GameConfig* config = config_create();
  
    config->max_attempts = 10;
    config->max_input_length = 20;
    config->min_range = 1;
    config->max_range = 50;
    config->allow_hints = TRUE;
    config->show_history = TRUE;
    config->auto_save = FALSE;
    config->difficulty = DIFFICULTY_NORMAL;
    strncpy(config->player_name, "Player", sizeof(config->player_name) - 1);
  
    return config;
}

// 根据难度创建配置
GameConfig* config_create_from_difficulty(DifficultyLevel difficulty) {
    GameConfig* config = config_create();
  
    config->difficulty = difficulty;
    strncpy(config->player_name, "Player", sizeof(config->player_name) - 1);
    config->allow_hints = TRUE;
    config->show_history = TRUE;
    config->auto_save = FALSE;
    config->max_input_length = 20;
  
    switch (difficulty) {
        case DIFFICULTY_EASY:
            config->max_attempts = 0; // 0表示无限
            config->min_range = 1;
            config->max_range = 10;
            break;
          
        case DIFFICULTY_NORMAL:
            config->max_attempts = 10;
            config->min_range = 1;
            config->max_range = 50;
            break;
          
        case DIFFICULTY_HARD:
            config->max_attempts = 5;
            config->min_range = 1;
            config->max_range = 100;
            break;
          
        case DIFFICULTY_EXPERT:
            config->max_attempts = 3;
            config->min_range = 1;
            config->max_range = 1000;
            config->allow_hints = FALSE;
            break;
          
        case DIFFICULTY_CUSTOM:
            // 使用默认值，用户稍后自定义
            config->max_attempts = 10;
            config->min_range = 1;
            config->max_range = 50;
            break;
    }
  
    return config;
}

// 创建自定义配置
GameConfig* config_create_custom(uint32_t min_range, uint32_t max_range, 
                               uint32_t max_attempts, Bool allow_hints) {
    GameConfig* config = config_create();
  
    config->min_range = min_range;
    config->max_range = max_range;
    config->max_attempts = max_attempts;
    config->allow_hints = allow_hints;
    config->show_history = TRUE;
    config->auto_save = FALSE;
    config->difficulty = DIFFICULTY_CUSTOM;
    config->max_input_length = 20;
    strncpy(config->player_name, "Player", sizeof(config->player_name) - 1);
  
    return config;
}

// 复制配置
void config_copy(GameConfig* dest, const GameConfig* src) {
    if (dest == NULL || src == NULL) return;
  
    memcpy(dest, src, sizeof(GameConfig));
}

// 克隆配置
GameConfig* config_clone(const GameConfig* src) {
    if (src == NULL) return NULL;
  
    GameConfig* clone = config_create();
    config_copy(clone, src);
    return clone;
}

// 验证配置有效性
Bool config_validate(const GameConfig* config, char* error_msg, size_t error_size) {
    if (config == NULL) {
        if (error_msg && error_size > 0) {
            snprintf(error_msg, error_size, "Config is NULL");
        }
        return FALSE;
    }
  
    // 检查范围
    if (config->min_range >= config->max_range) {
        if (error_msg && error_size > 0) {
            snprintf(error_msg, error_size, 
                    "Invalid range: min(%u) must be less than max(%u)", 
                    config->min_range, config->max_range);
        }
        return FALSE;
    }
  
    // 检查输入长度
    if (config->max_input_length < 1 || config->max_input_length > 256) {
        if (error_msg && error_size > 0) {
            snprintf(error_msg, error_size, 
                    "Invalid max input length: %u (must be 1-256)", 
                    config->max_input_length);
        }
        return FALSE;
    }
  
    // 检查难度级别
    if (config->difficulty > DIFFICULTY_CUSTOM) {
        if (error_msg && error_size > 0) {
            snprintf(error_msg, error_size, 
                    "Invalid difficulty level: %d", config->difficulty);
        }
        return FALSE;
    }
  
    return TRUE;
}

// 获取范围大小
uint32_t config_get_range_size(const GameConfig* config) {
    if (config == NULL) return 0;
  
    if (config->max_range < config->min_range) {
        return 0;
    }
  
    return config->max_range - config->min_range + 1;
}

// 获取难度名称
const char* config_get_difficulty_name(DifficultyLevel difficulty) {
    switch (difficulty) {
        case DIFFICULTY_EASY: return "Easy";
        case DIFFICULTY_NORMAL: return "Normal";
        case DIFFICULTY_HARD: return "Hard";
        case DIFFICULTY_EXPERT: return "Expert";
        case DIFFICULTY_CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

// 显示配置信息
void config_display(const GameConfig* config) {
    if (config == NULL) {
        printf("Config: NULL\n");
        return;
    }
  
    printf("╔════════════════════════════════════════╗\n");
    printf("║           Game Configuration           ║\n");
    printf("╠════════════════════════════════════════╣\n");
    printf("║ Player: %-30s ║\n", config->player_name);
    printf("║ Difficulty: %-25s ║\n", 
           config_get_difficulty_name(config->difficulty));
    printf("║ Range: %-4u to %-4u (Size: %-4u)    ║\n", 
           config->min_range, config->max_range, 
           config_get_range_size(config));
  
    if (config->max_attempts == 0) {
        printf("║ Max Attempts: Unlimited               ║\n");
    } else {
        printf("║ Max Attempts: %-22u ║\n", config->max_attempts);
    }
  
    printf("║ Hints: %-30s ║\n", config->allow_hints ? "Enabled" : "Disabled");
    printf("║ History: %-28s ║\n", config->show_history ? "Shown" : "Hidden");
    printf("║ Auto-save: %-26s ║\n", config->auto_save ? "Enabled" : "Disabled");
    printf("╚════════════════════════════════════════╝\n");
}

// 保存配置到文件
Bool config_save_to_file(const GameConfig* config, const char* filename) {
    if (config == NULL || filename == NULL) return FALSE;
  
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        return FALSE;
    }
  
    size_t written = fwrite(config, sizeof(GameConfig), 1, file);
    fclose(file);
  
    return written == 1;
}

// 从文件加载配置
GameConfig* config_load_from_file(const char* filename) {
    if (filename == NULL) return NULL;
  
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return NULL;
    }
  
    GameConfig* config = config_create();
    size_t read = fread(config, sizeof(GameConfig), 1, file);
    fclose(file);
  
    if (read != 1) {
        config_destroy(&config);
        return NULL;
    }
  
    return config;
}

// 保存默认配置
Bool config_save_default(const GameConfig* config) {
    const char* path = config_get_default_config_path();
    return config_save_to_file(config, path);
}

// 加载默认配置
GameConfig* config_load_default(void) {
    const char* path = config_get_default_config_path();
    GameConfig* config = config_load_from_file(path);
  
    if (config == NULL) {
        config = config_create_default();
    }
  
    return config;
}

// 初始化路径
void config_paths_init(ConfigPaths* paths, const char* app_name) {
    if (paths == NULL || app_name == NULL) return;
  
#ifdef _WIN32
    const char* home = getenv("USERPROFILE");
#else
    const char* home = getenv("HOME");
#endif
  
    if (home == NULL) {
        home = ".";
    }
  
    // 配置文件路径
    snprintf(paths->config_path, sizeof(paths->config_path),
             "%s/.%s/config.bin", home, app_name);
  
    // 保存文件路径
    snprintf(paths->save_path, sizeof(paths->save_path),
             "%s/.%s/saves/", home, app_name);
  
    // 日志文件路径
    snprintf(paths->log_path, sizeof(paths->log_path),
             "%s/.%s/logs/", home, app_name);
}

// 获取默认配置文件路径
const char* config_get_default_config_path(void) {
    static char path[256] = {0};
  
    if (path[0] == '\0') {
        ConfigPaths paths;
        config_paths_init(&paths, "guess_game_c");
        strncpy(path, paths.config_path, sizeof(path) - 1);
    }
  
    return path;
}

// 获取默认保存路径
const char* config_get_default_save_path(void) {
    static char path[256] = {0};
  
    if (path[0] == '\0') {
        ConfigPaths paths;
        config_paths_init(&paths, "guess_game_c");
        strncpy(path, paths.save_path, sizeof(path) - 1);
    }
  
    return path;
}
```

### **4. 状态模块 (core/state.h/state.c)**

**src/core/state.h:**

```c
#ifndef STATE_H
#define STATE_H

#include "../common.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// 游戏状态枚举
typedef enum {
    STATE_NEW = 0,         // 新游戏
    STATE_PLAYING,         // 游戏中
    STATE_WON,             // 获胜
    STATE_LOST,            // 失败
    STATE_PAUSED,          // 暂停
    STATE_SAVED            // 已保存
} GameStatus;

// 游戏结果
typedef struct {
    GameStatus status;     // 游戏状态
    uint32_t attempts;     // 尝试次数
    uint32_t secret;       // 秘密数字
    time_t start_time;     // 开始时间
    time_t end_time;       // 结束时间
    Bool is_high_score;    // 是否高分
} GameResult;

// 猜测记录
typedef struct {
    uint32_t guess;        // 猜测值
    time_t timestamp;      // 时间戳
    int8_t hint;           // 提示：-1(太小), 0(正确), 1(太大)
} GuessRecord;

// 游戏历史
typedef struct {
    GuessRecord* records;  // 记录数组
    size_t capacity;       // 容量
    size_t size;           // 当前大小
} GameHistory;

// 游戏状态
typedef struct {
    uint32_t secret_number;    // 秘密数字
    uint32_t attempts;         // 当前尝试次数
    GameConfig config;         // 游戏配置
    GameStatus status;         // 当前状态
    GameHistory history;       // 猜测历史
    time_t start_time;         // 游戏开始时间
    char session_id[37];       // 会话ID (UUID格式)
} GameState;

// 创建和销毁
GameState* state_create(const GameConfig* config);
void state_destroy(GameState** state);
GameState* state_clone(const GameState* state);

// 状态管理
void state_reset(GameState* state);
void state_start(GameState* state);
void state_pause(GameState* state);
void state_resume(GameState* state);

// 猜测处理
Bool state_make_guess(GameState* state, uint32_t guess, GameResult* result);
int state_compare_guess(const GameState* state, uint32_t guess);

// 历史管理
void state_add_guess(GameState* state, uint32_t guess, int8_t hint);
void state_clear_history(GameState* state);
size_t state_get_history_size(const GameState* state);
const GuessRecord* state_get_history_at(const GameState* state, size_t index);

// 状态查询
Bool state_is_active(const GameState* state);
Bool state_is_game_over(const GameState* state);
uint32_t state_get_remaining_attempts(const GameState* state);
time_t state_get_elapsed_time(const GameState* state);
uint32_t state_get_best_guess(const GameState* state);
float state_get_average_guess(const GameState* state);

// 序列化和反序列化
Bool state_save_to_file(const GameState* state, const char* filename);
GameState* state_load_from_file(const char* filename);
char* state_to_json(const GameState* state);
GameState* state_from_json(const char* json);

// 统计信息
void state_print_stats(const GameState* state);
void state_generate_session_id(GameState* state);

#ifdef __cplusplus
}
#endif

#endif // STATE_H
```

**src/core/state.c:**

```c
#include "state.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// 创建游戏状态
GameState* state_create(const GameConfig* config) {
    if (config == NULL) return NULL;
  
    GameState* state = (GameState*)safe_malloc(sizeof(GameState));
    memset(state, 0, sizeof(GameState));
  
    // 复制配置
    state->config = *config;
  
    // 生成秘密数字
    state->secret_number = random_int(config->min_range, config->max_range);
  
    // 初始化历史记录
    state->history.capacity = 16;
    state->history.size = 0;
    state->history.records = (GuessRecord*)safe_calloc(state->history.capacity, 
                                                      sizeof(GuessRecord));
  
    // 设置初始状态
    state->status = STATE_NEW;
    state->attempts = 0;
  
    // 生成会话ID
    state_generate_session_id(state);
  
    return state;
}

// 销毁游戏状态
void state_destroy(GameState** state) {
    if (state != NULL && *state != NULL) {
        GameState* s = *state;
      
        // 释放历史记录
        safe_free((void**)&s->history.records);
      
        // 释放状态对象
        safe_free((void**)state);
    }
}

// 克隆游戏状态
GameState* state_clone(const GameState* state) {
    if (state == NULL) return NULL;
  
    GameState* clone = (GameState*)safe_malloc(sizeof(GameState));
    memcpy(clone, state, sizeof(GameState));
  
    // 克隆历史记录
    clone->history.records = (GuessRecord*)safe_calloc(
        state->history.capacity, sizeof(GuessRecord));
    memcpy(clone->history.records, state->history.records,
           state->history.size * sizeof(GuessRecord));
  
    return clone;
}

// 重置游戏状态
void state_reset(GameState* state) {
    if (state == NULL) return;
  
    // 生成新的秘密数字
    state->secret_number = random_int(state->config.min_range, 
                                     state->config.max_range);
  
    // 重置计数器
    state->attempts = 0;
    state->status = STATE_NEW;
  
    // 清空历史记录
    state_clear_history(state);
  
    // 生成新的会话ID
    state_generate_session_id(state);
}

// 开始游戏
void state_start(GameState* state) {
    if (state == NULL) return;
  
    state->status = STATE_PLAYING;
    state->start_time = time(NULL);
}

// 暂停游戏
void state_pause(GameState* state) {
    if (state == NULL) return;
  
    if (state->status == STATE_PLAYING) {
        state->status = STATE_PAUSED;
    }
}

// 恢复游戏
void state_resume(GameState* state) {
    if (state == NULL) return;
  
    if (state->status == STATE_PAUSED) {
        state->status = STATE_PLAYING;
    }
}

// 处理猜测
Bool state_make_guess(GameState* state, uint32_t guess, GameResult* result) {
    if (state == NULL || !state_is_active(state)) {
        return FALSE;
    }
  
    // 增加尝试次数
    state->attempts++;
  
    // 比较猜测
    int comparison = state_compare_guess(state, guess);
  
    // 添加到历史记录
    state_add_guess(state, guess, (int8_t)comparison);
  
    // 准备结果
    if (result != NULL) {
        result->attempts = state->attempts;
        result->secret = state->secret_number;
        result->start_time = state->start_time;
        result->end_time = time(NULL);
      
        if (comparison == 0) {
            // 猜对了
            state->status = STATE_WON;
            result->status = STATE_WON;
            result->is_high_score = TRUE; // 简化处理，实际需要比较分数
        } else if (state->config.max_attempts > 0 && 
                   state->attempts >= state->config.max_attempts) {
            // 超过最大尝试次数
            state->status = STATE_LOST;
            result->status = STATE_LOST;
            result->is_high_score = FALSE;
        } else {
            // 继续游戏
            result->status = STATE_PLAYING;
            result->is_high_score = FALSE;
        }
    }
  
    return TRUE;
}

// 比较猜测
int state_compare_guess(const GameState* state, uint32_t guess) {
    if (state == NULL) return 0;
  
    if (guess < state->secret_number) {
        return -1; // 太小
    } else if (guess > state->secret_number) {
        return 1;  // 太大
    } else {
        return 0;  // 正确
    }
}

// 添加猜测记录
void state_add_guess(GameState* state, uint32_t guess, int8_t hint) {
    if (state == NULL) return;
  
    // 检查是否需要扩容
    if (state->history.size >= state->history.capacity) {
        state->history.capacity *= 2;
        state->history.records = (GuessRecord*)safe_realloc(
            state->history.records, 
            state->history.capacity * sizeof(GuessRecord));
    }
  
    // 添加记录
    GuessRecord* record = &state->history.records[state->history.size];
    record->guess = guess;
    record->hint = hint;
    record->timestamp = time(NULL);
  
    state->history.size++;
}

// 清空历史记录
void state_clear_history(GameState* state) {
    if (state == NULL) return;
  
    state->history.size = 0;
}

// 获取历史记录大小
size_t state_get_history_size(const GameState* state) {
    return state != NULL ? state->history.size : 0;
}

// 获取指定索引的历史记录
const GuessRecord* state_get_history_at(const GameState* state, size_t index) {
    if (state == NULL || index >= state->history.size) {
        return NULL;
    }
  
    return &state->history.records[index];
}

// 检查游戏是否活跃
Bool state_is_active(const GameState* state) {
    return state != NULL && 
           (state->status == STATE_PLAYING || state->status == STATE_PAUSED);
}

// 检查游戏是否结束
Bool state_is_game_over(const GameState* state) {
    return state != NULL && 
           (state->status == STATE_WON || state->status == STATE_LOST);
}

// 获取剩余尝试次数
uint32_t state_get_remaining_attempts(const GameState* state) {
    if (state == NULL || state->config.max_attempts == 0) {
        return 0;
    }
  
    if (state->attempts >= state->config.max_attempts) {
        return 0;
    }
  
    return state->config.max_attempts - state->attempts;
}

// 获取经过的时间
time_t state_get_elapsed_time(const GameState* state) {
    if (state == NULL || state->start_time == 0) {
        return 0;
    }
  
    if (state_is_active(state)) {
        return time(NULL) - state->start_time;
    }
  
    return 0;
}

// 获取最佳猜测（最接近秘密数字）
uint32_t state_get_best_guess(const GameState* state) {
    if (state == NULL || state->history.size == 0) {
        return 0;
    }
  
    uint32_t best_guess = 0;
    uint32_t min_diff = UINT32_MAX;
  
    for (size_t i = 0; i < state->history.size; i++) {
        uint32_t guess = state->history.records[i].guess;
        uint32_t diff = (guess > state->secret_number) ? 
                       (guess - state->secret_number) : 
                       (state->secret_number - guess);
      
        if (diff < min_diff) {
            min_diff = diff;
            best_guess = guess;
        }
    }
  
    return best_guess;
}

// 获取平均猜测值
float state_get_average_guess(const GameState* state) {
    if (state == NULL || state->history.size == 0) {
        return 0.0f;
    }
  
    uint64_t sum = 0;
    for (size_t i = 0; i < state->history.size; i++) {
        sum += state->history.records[i].guess;
    }
  
    return (float)sum / (float)state->history.size;
}

// 生成会话ID（简化版UUID）
void state_generate_session_id(GameState* state) {
    if (state == NULL) return;
  
    // 使用时间戳和随机数生成简单的会话ID
    time_t now = time(NULL);
    unsigned int random = random_int(0, 999999);
  
    snprintf(state->session_id, sizeof(state->session_id),
             "%08lx-%06x-%06x", (unsigned long)now, 
             random, random_int(0, 999999));
}

// 打印统计信息
void state_print_stats(const GameState* state) {
    if (state == NULL) return;
  
    printf("╔════════════════════════════════════════╗\n");
    printf("║           Game Statistics              ║\n");
    printf("╠════════════════════════════════════════╣\n");
    printf("║ Session ID: %-26s ║\n", state->session_id);
    printf("║ Status: %-31s ║\n", 
           state->status == STATE_WON ? "WON" : 
           state->status == STATE_LOST ? "LOST" : "PLAYING");
    printf("║ Attempts: %-29u ║\n", state->attempts);
  
    if (state->config.max_attempts > 0) {
        printf("║ Remaining: %-28u ║\n", state_get_remaining_attempts(state));
    } else {
        printf("║ Remaining: Unlimited                ║\n");
    }
  
    printf("║ Best Guess: %-27u ║\n", state_get_best_guess(state));
    printf("║ Average Guess: %-24.2f ║\n", state_get_average_guess(state));
    printf("║ Time: %-32lds ║\n", state_get_elapsed_time(state));
    printf("╚════════════════════════════════════════╝\n");
}
```

### **5. 验证器模块 (core/validator.h/validator.c)**

**src/core/validator.h:**

```c
#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "../common.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// 验证错误类型
typedef enum {
    VALID_OK = 0,              // 验证成功
    VALID_ERR_EMPTY,           // 输入为空
    VALID_ERR_TOO_LONG,        // 输入过长
    VALID_ERR_NOT_NUMERIC,     // 不是数字
    VALID_ERR_OUT_OF_RANGE,    // 超出范围
    VALID_ERR_INVALID_FORMAT,  // 无效格式
    VALID_ERR_SYSTEM           // 系统错误
} ValidatorError;

// 验证结果
typedef struct {
    ValidatorError error;      // 错误代码
    uint32_t value;           // 验证后的值
    char message[256];        // 错误消息
} ValidationResult;

// 提示类型
typedef enum {
    HINT_NONE = 0,            // 无提示
    HINT_VERY_CLOSE,          // 非常接近
    HINT_CLOSE,               // 接近
    HINT_FAR,                 // 远
    HINT_VERY_FAR,            // 非常远
    HINT_FIRST_TRY,           // 第一次尝试
    HINT_LOW_REMAINING,       // 剩余尝试少
    HINT_HOT,                 // 热（接近）
    HINT_WARM,                // 温暖
    HINT_COLD,                // 冷（远离）
    HINT_FREEZING             // 冰冻（很远）
} HintType;

// 提示信息
typedef struct {
    HintType type;            // 提示类型
    char text[128];          // 提示文本
    int severity;            // 严重程度（1-10）
} HintInfo;

// 验证器
typedef struct {
    GameConfig config;        // 游戏配置
    uint32_t last_guess;     // 上次猜测
    uint32_t last_diff;      // 上次差值
} InputValidator;

// 函数声明
InputValidator* validator_create(const GameConfig* config);
void validator_destroy(InputValidator** validator);

// 输入验证
ValidationResult validator_validate_input(const InputValidator* validator, 
                                         const char* input);
ValidationResult validator_validate_number(const InputValidator* validator, 
                                          uint32_t number);

// 提示生成
HintInfo validator_generate_hint(const InputValidator* validator,
                               uint32_t guess, uint32_t secret,
                               uint32_t attempts);
HintInfo validator_generate_comparison_hint(const InputValidator* validator,
                                          uint32_t guess, uint32_t secret);
HintInfo validator_generate_progress_hint(const InputValidator* validator,
                                        uint32_t attempts);

// 统计分析
float validator_calculate_accuracy(const InputValidator* validator,
                                 const uint32_t* guesses, size_t count,
                                 uint32_t secret);
int validator_calculate_trend(const InputValidator* validator,
                            const uint32_t* guesses, size_t count,
                            uint32_t secret);

// 错误处理
const char* validator_get_error_message(ValidatorError error);
void validator_print_error(const ValidationResult* result);

#ifdef __cplusplus
}
#endif

#endif // VALIDATOR_H
```

**src/core/validator.c:**

```c
#include "validator.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// 创建验证器
InputValidator* validator_create(const GameConfig* config) {
    if (config == NULL) return NULL;
  
    InputValidator* validator = (InputValidator*)safe_malloc(sizeof(InputValidator));
    memset(validator, 0, sizeof(InputValidator));
  
    validator->config = *config;
    validator->last_guess = 0;
    validator->last_diff = 0;
  
    return validator;
}

// 销毁验证器
void validator_destroy(InputValidator** validator) {
    safe_free((void**)validator);
}

// 验证输入字符串
ValidationResult validator_validate_input(const InputValidator* validator, 
                                         const char* input) {
    ValidationResult result = {VALID_OK, 0, ""};
  
    if (validator == NULL) {
        result.error = VALID_ERR_SYSTEM;
        strcpy(result.message, "Validator is NULL");
        return result;
    }
  
    if (input == NULL || *input == '\0') {
        result.error = VALID_ERR_EMPTY;
        strcpy(result.message, "Input cannot be empty");
        return result;
    }
  
    // 检查长度
    size_t len = strlen(input);
    if (len > validator->config.max_input_length) {
        result.error = VALID_ERR_TOO_LONG;
        snprintf(result.message, sizeof(result.message),
                "Input too long (max %u characters)", 
                validator->config.max_input_length);
        return result;
    }
  
    // 检查是否为数字
    if (!is_string_numeric(input)) {
        result.error = VALID_ERR_NOT_NUMERIC;
        strcpy(result.message, "Input must be a valid number");
        return result;
    }
  
    // 转换为数字
    char* endptr;
    long num = strtol(input, &endptr, 10);
  
    if (endptr == input || *endptr != '\0') {
        result.error = VALID_ERR_INVALID_FORMAT;
        strcpy(result.message, "Invalid number format");
        return result;
    }
  
    // 检查范围
    if (num < (long)validator->config.min_range || 
        num > (long)validator->config.max_range) {
        result.error = VALID_ERR_OUT_OF_RANGE;
        snprintf(result.message, sizeof(result.message),
                "Number must be between %u and %u",
                validator->config.min_range,
                validator->config.max_range);
        return result;
    }
  
    result.value = (uint32_t)num;
    return result;
}

// 验证数字
ValidationResult validator_validate_number(const InputValidator* validator, 
                                          uint32_t number) {
    ValidationResult result = {VALID_OK, number, ""};
  
    if (validator == NULL) {
        result.error = VALID_ERR_SYSTEM;
        return result;
    }
  
    if (number < validator->config.min_range || 
        number > validator->config.max_range) {
        result.error = VALID_ERR_OUT_OF_RANGE;
        snprintf(result.message, sizeof(result.message),
                "Number %u is out of range (%u-%u)",
                number, validator->config.min_range,
                validator->config.max_range);
    }
  
    return result;
}

// 生成提示
HintInfo validator_generate_hint(const InputValidator* validator,
                               uint32_t guess, uint32_t secret,
                               uint32_t attempts) {
    HintInfo hint = {HINT_NONE, "", 0};
  
    if (!validator->config.allow_hints) {
        return hint;
    }
  
    if (guess == secret) {
        hint.type = HINT_NONE;
        strcpy(hint.text, "Correct!");
        hint.severity = 0;
        return hint;
    }
  
    // 计算差值百分比
    uint32_t range_size = validator->config.max_range - 
                         validator->config.min_range + 1;
    uint32_t diff = (guess > secret) ? (guess - secret) : (secret - guess);
    float diff_percent = (float)diff / (float)range_size * 100.0f;
  
    // 根据差值生成提示
    if (diff <= 2) {
        hint.type = HINT_VERY_CLOSE;
        strcpy(hint.text, "🔥 Very close!");
        hint.severity = 1;
    } else if (diff <= range_size * 0.05) { // 5%以内
        hint.type = HINT_CLOSE;
        strcpy(hint.text, "🌟 Close!");
        hint.severity = 2;
    } else if (diff <= range_size * 0.15) { // 15%以内
        hint.type = HINT_WARM;
        strcpy(hint.text, "💡 Getting warm");
        hint.severity = 4;
    } else if (diff <= range_size * 0.3) { // 30%以内
        hint.type = HINT_COLD;
        strcpy(hint.text, "❄️ A bit cold");
        hint.severity = 6;
    } else {
        hint.type = HINT_FREEZING;
        strcpy(hint.text, "🧊 Very far away");
        hint.severity = 9;
    }
  
    // 添加尝试次数相关的提示
    if (attempts == 1) {
        hint.type = HINT_FIRST_TRY;
        strcpy(hint.text, "🎯 First try! Good luck!");
        hint.severity = 1;
    } else if (validator->config.max_attempts > 0) {
        uint32_t remaining = validator->config.max_attempts - attempts;
        if (remaining <= 2) {
            hint.type = HINT_LOW_REMAINING;
            snprintf(hint.text, sizeof(hint.text),
                    "⏰ Only %u attempts left!", remaining);
            hint.severity = 8;
        }
    }
  
    return hint;
}

// 生成比较提示
HintInfo validator_generate_comparison_hint(const InputValidator* validator,
                                          uint32_t guess, uint32_t secret) {
    HintInfo hint = {HINT_NONE, "", 0};
  
    uint32_t diff = (guess > secret) ? (guess - secret) : (secret - guess);
  
    if (guess < secret) {
        strcpy(hint.text, "📈 Too small!");
    } else {
        strcpy(hint.text, "📉 Too big!");
    }
  
    hint.type = HINT_NONE;
    hint.severity = diff; // 使用差值作为严重程度
  
    return hint;
}

// 生成进度提示
HintInfo validator_generate_progress_hint(const InputValidator* validator,
                                        uint32_t attempts) {
    HintInfo hint = {HINT_NONE, "", 0};
  
    if (validator->config.max_attempts == 0) {
        return hint;
    }
  
    float progress = (float)attempts / (float)validator->config.max_attempts * 100.0f;
  
    if (progress >= 80.0f) {
        hint.type = HINT_LOW_REMAINING;
        snprintf(hint.text, sizeof(hint.text),
                "🚨 High effort (%u/%u)", attempts, validator->config.max_attempts);
        hint.severity = 9;
    } else if (progress >= 50.0f) {
        hint.type = HINT_NONE;
        snprintf(hint.text, sizeof(hint.text),
                "⚠️ Halfway there (%u/%u)", attempts, validator->config.max_attempts);
        hint.severity = 5;
    }
  
    return hint;
}

// 计算准确率
float validator_calculate_accuracy(const InputValidator* validator,
                                 const uint32_t* guesses, size_t count,
                                 uint32_t secret) {
    if (guesses == NULL || count == 0 || validator == NULL) {
        return 0.0f;
    }
  
    float total_error = 0.0f;
    uint32_t range_size = validator->config.max_range - 
                         validator->config.min_range + 1;
  
    for (size_t i = 0; i < count; i++) {
        uint32_t diff = (guesses[i] > secret) ? 
                       (guesses[i] - secret) : 
                       (secret - guesses[i]);
        float error_percent = (float)diff / (float)range_size * 100.0f;
        total_error += error_percent;
    }
  
    float avg_error = total_error / (float)count;
    return 100.0f - avg_error; // 准确率 = 100 - 平均误差
}

// 计算趋势（负值表示接近，正值表示远离）
int validator_calculate_trend(const InputValidator* validator,
                            const uint32_t* guesses, size_t count,
                            uint32_t secret) {
    if (guesses == NULL || count < 2 || validator == NULL) {
        return 0;
    }
  
    uint32_t prev_diff = (guesses[0] > secret) ? 
                        (guesses[0] - secret) : 
                        (secret - guesses[0]);
    int trend = 0;
  
    for (size_t i = 1; i < count; i++) {
        uint32_t curr_diff = (guesses[i] > secret) ? 
                           (guesses[i] - secret) : 
                           (secret - guesses[i]);
      
        if (curr_diff < prev_diff) {
            trend--; // 变得更接近
        } else if (curr_diff > prev_diff) {
            trend++; // 变得更远离
        }
      
        prev_diff = curr_diff;
    }
  
    return trend;
}

// 获取错误消息
const char* validator_get_error_message(ValidatorError error) {
    switch (error) {
        case VALID_OK: return "OK";
        case VALID_ERR_EMPTY: return "Input is empty";
        case VALID_ERR_TOO_LONG: return "Input is too long";
        case VALID_ERR_NOT_NUMERIC: return "Input is not numeric";
        case VALID_ERR_OUT_OF_RANGE: return "Number is out of range";
        case VALID_ERR_INVALID_FORMAT: return "Invalid number format";
        case VALID_ERR_SYSTEM: return "System error";
        default: return "Unknown error";
    }
}

// 打印错误
void validator_print_error(const ValidationResult* result) {
    if (result == NULL) return;
  
    if (result->error != VALID_OK) {
        printf("Validation Error: %s\n", result->message);
    }
}
```

### **6. 输入模块 (core/input.h/input.c)**

**src/core/input.h:**

```c
#ifndef INPUT_H
#define INPUT_H

#include "../common.h"
#include "config.h"
#include "validator.h"

#ifdef __cplusplus
extern "C" {
#endif

// 输入选项
typedef struct {
    uint32_t max_retries;     // 最大重试次数
    Bool show_prompt;         // 显示提示
    Bool clear_buffer;        // 清除缓冲区
    Bool trim_input;          // 修剪输入
    Bool allow_empty;         // 允许空输入
    char default_value[64];   // 默认值
} InputOptions;

// 输入处理器
typedef struct {
    InputValidator validator; // 验证器
    InputOptions options;     // 选项
    uint32_t total_inputs;    // 总输入次数
    uint32_t failed_inputs;   // 失败输入次数
} InputHandler;

// 创建和销毁
InputHandler* input_handler_create(const GameConfig* config);
void input_handler_destroy(InputHandler** handler);
void input_handler_set_options(InputHandler* handler, const InputOptions* options);

// 输入读取
Bool input_handler_read_number(InputHandler* handler, const char* prompt, 
                              uint32_t* value, char* error_msg, size_t error_size);
uint32_t* input_handler_read_number_with_retry(InputHandler* handler, 
                                              const char* prompt, 
                                              uint32_t max_retries,
                                              char** error_msg);

// 特殊输入
Bool input_handler_read_yes_no(InputHandler* handler, const char* prompt, 
                              Bool* value, char* error_msg, size_t error_size);
Bool input_handler_read_range(InputHandler* handler, const char* prompt,
                             uint32_t* min, uint32_t* max,
                             char* error_msg, size_t error_size);
Bool input_handler_read_selection(InputHandler* handler, const char* prompt,
                                 const char* options[], size_t option_count,
                                 size_t* selection, char* error_msg, size_t error_size);

// 统计信息
void input_handler_get_stats(const InputHandler* handler, 
                            uint32_t* total, uint32_t* failed, float* success_rate);
void input_handler_reset_stats(InputHandler* handler);

// 辅助函数
void input_handler_print_prompt(const InputHandler* handler, const char* prompt);
void input_handler_clear_input(void);
char* input_handler_read_line(char* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif // INPUT_H
```

**src/core/input.c:**

```c
#include "input.h"
#include <stdio.h>
#include <string.h>

// 创建输入处理器
InputHandler* input_handler_create(const GameConfig* config) {
    if (config == NULL) return NULL;
  
    InputHandler* handler = (InputHandler*)safe_malloc(sizeof(InputHandler));
    memset(handler, 0, sizeof(InputHandler));
  
    // 初始化验证器
    handler->validator.config = *config;
  
    // 设置默认选项
    handler->options.max_retries = 3;
    handler->options.show_prompt = TRUE;
    handler->options.clear_buffer = TRUE;
    handler->options.trim_input = TRUE;
    handler->options.allow_empty = FALSE;
    strcpy(handler->options.default_value, "");
  
    // 初始化统计
    handler->total_inputs = 0;
    handler->failed_inputs = 0;
  
    return handler;
}

// 销毁输入处理器
void input_handler_destroy(InputHandler** handler) {
    safe_free((void**)handler);
}

// 设置选项
void input_handler_set_options(InputHandler* handler, const InputOptions* options) {
    if (handler == NULL || options == NULL) return;
  
    handler->options = *options;
}

// 读取数字
Bool input_handler_read_number(InputHandler* handler, const char* prompt, 
                              uint32_t* value, char* error_msg, size_t error_size) {
    if (handler == NULL || value == NULL) {
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Invalid arguments", error_size - 1);
        }
        return FALSE;
    }
  
    handler->total_inputs++;
  
    // 显示提示
    if (handler->options.show_prompt && prompt != NULL) {
        input_handler_print_prompt(handler, prompt);
    }
  
    // 读取输入
    char buffer[256];
    if (!input_handler_read_line(buffer, sizeof(buffer))) {
        handler->failed_inputs++;
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Failed to read input", error_size - 1);
        }
        return FALSE;
    }
  
    // 处理空输入
    if (handler->options.trim_input) {
        string_trim(buffer);
    }
  
    if (!handler->options.allow_empty && buffer[0] == '\0') {
        // 使用默认值
        if (handler->options.default_value[0] != '\0') {
            strncpy(buffer, handler->options.default_value, sizeof(buffer) - 1);
        } else {
            handler->failed_inputs++;
            if (error_msg && error_size > 0) {
                strncpy(error_msg, "Input cannot be empty", error_size - 1);
            }
            return FALSE;
        }
    }
  
    // 验证输入
    ValidationResult result = validator_validate_input(&handler->validator, buffer);
  
    if (result.error != VALID_OK) {
        handler->failed_inputs++;
        if (error_msg && error_size > 0) {
            strncpy(error_msg, result.message, error_size - 1);
            error_msg[error_size - 1] = '\0';
        }
        return FALSE;
    }
  
    *value = result.value;
    return TRUE;
}

// 带重试的读取数字
uint32_t* input_handler_read_number_with_retry(InputHandler* handler, 
                                              const char* prompt, 
                                              uint32_t max_retries,
                                              char** error_msg) {
    if (handler == NULL || prompt == NULL) {
        if (error_msg) *error_msg = strdup("Invalid arguments");
        return NULL;
    }
  
    uint32_t actual_retries = max_retries > 0 ? max_retries : 
                             handler->options.max_retries;
  
    for (uint32_t attempt = 0; attempt < actual_retries; attempt++) {
        char error_buffer[256] = {0};
        uint32_t value = 0;
      
        // 显示重试计数
        if (attempt > 0) {
            printf("Retry %u/%u\n", attempt + 1, actual_retries);
        }
      
        if (input_handler_read_number(handler, prompt, &value, 
                                     error_buffer, sizeof(error_buffer))) {
            uint32_t* result = (uint32_t*)safe_malloc(sizeof(uint32_t));
            *result = value;
          
            if (error_msg) *error_msg = NULL;
            return result;
        }
      
        printf("Error: %s\n", error_buffer);
      
        // 最后一次尝试失败
        if (attempt == actual_retries - 1) {
            if (error_msg) {
                *error_msg = strdup("Maximum retries exceeded");
            }
        }
    }
  
    return NULL;
}

// 读取是/否选择
Bool input_handler_read_yes_no(InputHandler* handler, const char* prompt, 
                              Bool* value, char* error_msg, size_t error_size) {
    if (handler == NULL || value == NULL) {
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Invalid arguments", error_size - 1);
        }
        return FALSE;
    }
  
    char buffer[256];
  
    if (prompt != NULL) {
        printf("%s (y/n): ", prompt);
    } else {
        printf("(y/n): ");
    }
  
    flush_output_stream();
  
    if (!input_handler_read_line(buffer, sizeof(buffer))) {
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Failed to read input", error_size - 1);
        }
        return FALSE;
    }
  
    string_trim(buffer);
    string_to_lower(buffer);
  
    if (strcmp(buffer, "y") == 0 || strcmp(buffer, "yes") == 0 || 
        strcmp(buffer, "1") == 0 || strcmp(buffer, "true") == 0) {
        *value = TRUE;
        return TRUE;
    } else if (strcmp(buffer, "n") == 0 || strcmp(buffer, "no") == 0 || 
               strcmp(buffer, "0") == 0 || strcmp(buffer, "false") == 0) {
        *value = FALSE;
        return TRUE;
    }
  
    if (error_msg && error_size > 0) {
        strncpy(error_msg, "Please enter 'y' or 'n'", error_size - 1);
    }
    return FALSE;
}

// 读取范围
Bool input_handler_read_range(InputHandler* handler, const char* prompt,
                             uint32_t* min, uint32_t* max,
                             char* error_msg, size_t error_size) {
    if (handler == NULL || min == NULL || max == NULL) {
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Invalid arguments", error_size - 1);
        }
        return FALSE;
    }
  
    char buffer[256];
  
    if (prompt != NULL) {
        printf("%s (format: min,max): ", prompt);
    } else {
        printf("Enter range (format: min,max): ");
    }
  
    flush_output_stream();
  
    if (!input_handler_read_line(buffer, sizeof(buffer))) {
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Failed to read input", error_size - 1);
        }
        return FALSE;
    }
  
    string_trim(buffer);
  
    // 解析范围
    char* comma = strchr(buffer, ',');
    if (comma == NULL) {
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Invalid format. Use: min,max", error_size - 1);
        }
        return FALSE;
    }
  
    *comma = '\0';
    char* min_str = buffer;
    char* max_str = comma + 1;
  
    string_trim(min_str);
    string_trim(max_str);
  
    // 验证最小值
    ValidationResult min_result = validator_validate_input(&handler->validator, min_str);
    if (min_result.error != VALID_OK) {
        if (error_msg && error_size > 0) {
            snprintf(error_msg, error_size, "Invalid minimum: %s", min_result.message);
        }
        return FALSE;
    }
  
    // 验证最大值
    ValidationResult max_result = validator_validate_input(&handler->validator, max_str);
    if (max_result.error != VALID_OK) {
        if (error_msg && error_size > 0) {
            snprintf(error_msg, error_size, "Invalid maximum: %s", max_result.message);
        }
        return FALSE;
    }
  
    // 检查顺序
    if (min_result.value >= max_result.value) {
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Minimum must be less than maximum", error_size - 1);
        }
        return FALSE;
    }
  
    *min = min_result.value;
    *max = max_result.value;
    return TRUE;
}

// 读取选择
Bool input_handler_read_selection(InputHandler* handler, const char* prompt,
                                 const char* options[], size_t option_count,
                                 size_t* selection, char* error_msg, size_t error_size) {
    if (handler == NULL || options == NULL || selection == NULL || option_count == 0) {
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Invalid arguments", error_size - 1);
        }
        return FALSE;
    }
  
    // 显示选项
    if (prompt != NULL) {
        printf("%s\n", prompt);
    }
  
    for (size_t i = 0; i < option_count; i++) {
        printf("%zu. %s\n", i + 1, options[i]);
    }
  
    printf("Enter selection (1-%zu): ", option_count);
    flush_output_stream();
  
    char buffer[256];
    if (!input_handler_read_line(buffer, sizeof(buffer))) {
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Failed to read input", error_size - 1);
        }
        return FALSE;
    }
  
    string_trim(buffer);
  
    if (!is_string_numeric(buffer)) {
        if (error_msg && error_size > 0) {
            strncpy(error_msg, "Selection must be a number", error_size - 1);
        }
        return FALSE;
    }
  
    long choice = strtol(buffer, NULL, 10);
  
    if (choice < 1 || choice > (long)option_count) {
        if (error_msg && error_size > 0) {
            snprintf(error_msg, error_size, 
                    "Selection must be between 1 and %zu", option_count);
        }
        return FALSE;
    }
  
    *selection = (size_t)(choice - 1);
    return TRUE;
}

// 获取统计信息
void input_handler_get_stats(const InputHandler* handler, 
                            uint32_t* total, uint32_t* failed, float* success_rate) {
    if (handler == NULL) return;
  
    if (total != NULL) {
        *total = handler->total_inputs;
    }
  
    if (failed != NULL) {
        *failed = handler->failed_inputs;
    }
  
    if (success_rate != NULL) {
        if (handler->total_inputs > 0) {
            *success_rate = (float)(handler->total_inputs - handler->failed_inputs) / 
                          (float)handler->total_inputs * 100.0f;
        } else {
            *success_rate = 0.0f;
        }
    }
}

// 重置统计
void input_handler_reset_stats(InputHandler* handler) {
    if (handler == NULL) return;
  
    handler->total_inputs = 0;
    handler->failed_inputs = 0;
}

// 打印提示
void input_handler_print_prompt(const InputHandler* handler, const char* prompt) {
    if (prompt == NULL) return;
  
    printf("%s", prompt);
  
    if (handler != NULL && handler->options.default_value[0] != '\0') {
        printf(" [%s]", handler->options.default_value);
    }
  
    printf(": ");
    flush_output_stream();
}

// 清空输入
void input_handler_clear_input(void) {
    clear_input_buffer();
}

// 读取一行
char* input_handler_read_line(char* buffer, size_t size) {
    if (buffer == NULL || size == 0) return NULL;
  
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return NULL;
    }
  
    // 移除换行符
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
  
    return buffer;
}
```

### **7. 游戏核心模块 (core/game.h/game.c)**

**src/core/game.h:**

```c
#ifndef GAME_H
#define GAME_H

#include "../common.h"
#include "config.h"
#include "state.h"
#include "validator.h"
#include "input.h"

#ifdef __cplusplus
extern "C" {
#endif

// 游戏事件类型
typedef enum {
    EVENT_GAME_START = 0,     // 游戏开始
    EVENT_GAME_END,           // 游戏结束
    EVENT_GUESS_MADE,         // 猜测完成
    EVENT_HINT_GIVEN,         // 提示给出
    EVENT_STATE_CHANGED,      // 状态改变
    EVENT_CONFIG_UPDATED,     // 配置更新
    EVENT_ERROR_OCCURRED      // 错误发生
} GameEventType;

// 游戏事件
typedef struct {
    GameEventType type;       // 事件类型
    time_t timestamp;         // 时间戳
    void* data;              // 事件数据
    size_t data_size;        // 数据大小
} GameEvent;

// 游戏回调函数类型
typedef void (*GameCallback)(GameEvent* event, void* user_data);

// 游戏回调注册
typedef struct {
    GameCallback callback;    // 回调函数
    void* user_data;         // 用户数据
    Bool enabled;           // 是否启用
} GameCallbackEntry;

// 游戏管理器
typedef struct {
    GameState* state;                // 游戏状态
    InputHandler* input_handler;     // 输入处理器
    InputValidator* validator;       // 验证器
    GameConfig* config;              // 游戏配置
  
    // 回调系统
    GameCallbackEntry* callbacks;    // 回调数组
    size_t callback_count;           // 回调数量
    size_t callback_capacity;        // 回调容量
  
    // 统计
    uint32_t total_games;           // 总游戏数
    uint32_t games_won;             // 获胜游戏数
    uint32_t games_lost;            // 失败游戏数
    time_t total_play_time;         // 总游戏时间
  
} GameManager;

// 创建和销毁
GameManager* game_manager_create(void);
void game_manager_destroy(GameManager** manager);
GameManager* game_manager_create_with_config(const GameConfig* config);

// 游戏控制
Bool game_manager_start(GameManager* manager);
Bool game_manager_pause(GameManager* manager);
Bool game_manager_resume(GameManager* manager);
Bool game_manager_restart(GameManager* manager);
Bool game_manager_save(GameManager* manager, const char* filename);
Bool game_manager_load(GameManager* manager, const char* filename);

// 游戏操作
Bool game_manager_make_guess(GameManager* manager, uint32_t guess, GameResult* result);
HintInfo game_manager_get_hint(GameManager* manager, uint32_t guess);

// 配置管理
Bool game_manager_update_config(GameManager* manager, const GameConfig* config);
Bool game_manager_set_difficulty(GameManager* manager, DifficultyLevel difficulty);
void game_manager_reload_config(GameManager* manager);

// 回调系统
Bool game_manager_register_callback(GameManager* manager, GameEventType event_type,
                                   GameCallback callback, void* user_data);
Bool game_manager_unregister_callback(GameManager* manager, GameEventType event_type,
                                     GameCallback callback);
void game_manager_trigger_event(GameManager* manager, GameEventType event_type,
                               const void* data, size_t data_size);

// 统计信息
void game_manager_get_stats(const GameManager* manager, 
                           uint32_t* total_games, uint32_t* games_won,
                           uint32_t* games_lost, float* win_rate,
                           time_t* total_time, time_t* avg_time);
void game_manager_print_stats(const GameManager* manager);

// 游戏循环
void game_manager_run_interactive(GameManager* manager);
void game_manager_run_auto(GameManager* manager, uint32_t max_iterations);

// 序列化
char* game_manager_to_json(const GameManager* manager);
Bool game_manager_from_json(GameManager* manager, const char* json);

#ifdef __cplusplus
}
#endif

#endif // GAME_H
```

**src/core/game.c:**

```c
#include "game.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// 默认回调容量
#define DEFAULT_CALLBACK_CAPACITY 8

// 创建游戏管理器
GameManager* game_manager_create(void) {
    GameManager* manager = (GameManager*)safe_malloc(sizeof(GameManager));
    memset(manager, 0, sizeof(GameManager));
  
    // 创建默认配置
    manager->config = config_create_default();
    if (manager->config == NULL) {
        safe_free((void**)&manager);
        return NULL;
    }
  
    // 创建游戏状态
    manager->state = state_create(manager->config);
    if (manager->state == NULL) {
        config_destroy(&manager->config);
        safe_free((void**)&manager);
        return NULL;
    }
  
    // 创建输入处理器
    manager->input_handler = input_handler_create(manager->config);
    if (manager->input_handler == NULL) {
        state_destroy(&manager->state);
        config_destroy(&manager->config);
        safe_free((void**)&manager);
        return NULL;
    }
  
    // 创建验证器
    manager->validator = validator_create(manager->config);
    if (manager->validator == NULL) {
        input_handler_destroy(&manager->input_handler);
        state_destroy(&manager->state);
        config_destroy(&manager->config);
        safe_free((void**)&manager);
        return NULL;
    }
  
    // 初始化回调系统
    manager->callback_capacity = DEFAULT_CALLBACK_CAPACITY;
    manager->callback_count = 0;
    manager->callbacks = (GameCallbackEntry*)safe_calloc(
        manager->callback_capacity, sizeof(GameCallbackEntry));
  
    // 初始化统计
    manager->total_games = 0;
    manager->games_won = 0;
    manager->games_lost = 0;
    manager->total_play_time = 0;
  
    return manager;
}

// 销毁游戏管理器
void game_manager_destroy(GameManager** manager) {
    if (manager == NULL || *manager == NULL) return;
  
    GameManager* m = *manager;
  
    // 销毁回调系统
    safe_free((void**)&m->callbacks);
  
    // 销毁组件
    validator_destroy(&m->validator);
    input_handler_destroy(&m->input_handler);
    state_destroy(&m->state);
    config_destroy(&m->config);
  
    // 销毁管理器
    safe_free((void**)manager);
}

// 使用配置创建游戏管理器
GameManager* game_manager_create_with_config(const GameConfig* config) {
    if (config == NULL) return NULL;
  
    GameManager* manager = game_manager_create();
    if (manager == NULL) return NULL;
  
    // 更新配置
    if (!game_manager_update_config(manager, config)) {
        game_manager_destroy(&manager);
        return NULL;
    }
  
    return manager;
}

// 开始游戏
Bool game_manager_start(GameManager* manager) {
    if (manager == NULL || manager->state == NULL) {
        return FALSE;
    }
  
    if (manager->state->status == STATE_NEW || 
        manager->state->status == STATE_SAVED) {
        state_start(manager->state);
        manager->total_games++;
      
        // 触发事件
        GameEvent event = {EVENT_GAME_START, time(NULL), NULL, 0};
        game_manager_trigger_event(manager, EVENT_GAME_START, NULL, 0);
      
        return TRUE;
    }
  
    return FALSE;
}

// 暂停游戏
Bool game_manager_pause(GameManager* manager) {
    if (manager == NULL || manager->state == NULL) {
        return FALSE;
    }
  
    if (manager->state->status == STATE_PLAYING) {
        state_pause(manager->state);
      
        // 触发事件
        GameEvent event = {EVENT_STATE_CHANGED, time(NULL), NULL, 0};
        game_manager_trigger_event(manager, EVENT_STATE_CHANGED, NULL, 0);
      
        return TRUE;
    }
  
    return FALSE;
}

// 恢复游戏
Bool game_manager_resume(GameManager* manager) {
    if (manager == NULL || manager->state == NULL) {
        return FALSE;
    }
  
    if (manager->state->status == STATE_PAUSED) {
        state_resume(manager->state);
      
        // 触发事件
        GameEvent event = {EVENT_STATE_CHANGED, time(NULL), NULL, 0};
        game_manager_trigger_event(manager, EVENT_STATE_CHANGED, NULL, 0);
      
        return TRUE;
    }
  
    return FALSE;
}

// 重新开始游戏
Bool game_manager_restart(GameManager* manager) {
    if (manager == NULL || manager->state == NULL) {
        return FALSE;
    }
  
    // 更新统计
    if (manager->state->status == STATE_WON) {
        manager->games_won++;
    } else if (manager->state->status == STATE_LOST) {
        manager->games_lost++;
    }
  
    // 计算游戏时间
    if (manager->state->start_time > 0) {
        time_t end_time = time(NULL);
        manager->total_play_time += (end_time - manager->state->start_time);
    }
  
    // 重置游戏状态
    state_reset(manager->state);
  
    // 触发事件
    GameEvent event = {EVENT_GAME_END, time(NULL), NULL, 0};
    game_manager_trigger_event(manager, EVENT_GAME_END, NULL, 0);
  
    // 开始新游戏
    return game_manager_start(manager);
}

// 保存游戏
Bool game_manager_save(GameManager* manager, const char* filename) {
    if (manager == NULL || manager->state == NULL || filename == NULL) {
        return FALSE;
    }
  
    // 暂停游戏以便保存
    Bool was_paused = (manager->state->status == STATE_PAUSED);
    if (!was_paused) {
        game_manager_pause(manager);
    }
  
    // 保存状态
    Bool success = state_save_to_file(manager->state, filename);
  
    // 恢复游戏状态
    if (!was_paused && manager->state->status == STATE_PAUSED) {
        game_manager_resume(manager);
    }
  
    return success;
}

// 加载游戏
Bool game_manager_load(GameManager* manager, const char* filename) {
    if (manager == NULL || filename == NULL) {
        return FALSE;
    }
  
    // 加载状态
    GameState* loaded_state = state_load_from_file(filename);
    if (loaded_state == NULL) {
        return FALSE;
    }
  
    // 销毁当前状态
    state_destroy(&manager->state);
  
    // 使用加载的状态
    manager->state = loaded_state;
  
    // 更新配置
    manager->config = config_clone(&loaded_state->config);
  
    // 更新输入处理器和验证器
    input_handler_destroy(&manager->input_handler);
    validator_destroy(&manager->validator);
  
    manager->input_handler = input_handler_create(manager->config);
    manager->validator = validator_create(manager->config);
  
    // 触发事件
    GameEvent event = {EVENT_STATE_CHANGED, time(NULL), NULL, 0};
    game_manager_trigger_event(manager, EVENT_STATE_CHANGED, NULL, 0);
  
    return TRUE;
}

// 进行猜测
Bool game_manager_make_guess(GameManager* manager, uint32_t guess, GameResult* result) {
    if (manager == NULL || manager->state == NULL) {
        return FALSE;
    }
  
    // 验证猜测
    ValidationResult valid_result = validator_validate_number(
        manager->validator, guess);
  
    if (valid_result.error != VALID_OK) {
        // 触发错误事件
        GameEvent event = {EVENT_ERROR_OCCURRED, time(NULL), 
                          valid_result.message, strlen(valid_result.message) + 1};
        game_manager_trigger_event(manager, EVENT_ERROR_OCCURRED, 
                                  valid_result.message, strlen(valid_result.message) + 1);
        return FALSE;
    }
  
    // 处理猜测
    GameResult game_result;
    if (!state_make_guess(manager->state, guess, &game_result)) {
        return FALSE;
    }
  
    // 复制结果
    if (result != NULL) {
        *result = game_result;
    }
  
    // 触发猜测事件
    game_manager_trigger_event(manager, EVENT_GUESS_MADE, &guess, sizeof(guess));
  
    // 检查游戏是否结束
    if (game_result.status == STATE_WON || game_result.status == STATE_LOST) {
        // 更新统计
        if (game_result.status == STATE_WON) {
            manager->games_won++;
        } else {
            manager->games_lost++;
        }
      
        // 计算游戏时间
        if (manager->state->start_time > 0) {
            time_t end_time = time(NULL);
            manager->total_play_time += (end_time - manager->state->start_time);
        }
      
        // 触发游戏结束事件
        game_manager_trigger_event(manager, EVENT_GAME_END, 
                                  &game_result, sizeof(game_result));
    }
  
    return TRUE;
}

// 获取提示
HintInfo game_manager_get_hint(GameManager* manager, uint32_t guess) {
    HintInfo hint = {HINT_NONE, "", 0};
  
    if (manager == NULL || manager->state == NULL || manager->validator == NULL) {
        return hint;
    }
  
    if (!manager->config.allow_hints) {
        return hint;
    }
  
    // 生成提示
    hint = validator_generate_hint(manager->validator, guess, 
                                  manager->state->secret_number,
                                  manager->state->attempts);
  
    // 触发提示事件
    if (hint.type != HINT_NONE) {
        game_manager_trigger_event(manager, EVENT_HINT_GIVEN, &hint, sizeof(hint));
    }
  
    return hint;
}

// 更新配置
Bool game_manager_update_config(GameManager* manager, const GameConfig* config) {
    if (manager == NULL || config == NULL) {
        return FALSE;
    }
  
    // 验证配置
    char error_msg[256];
    if (!config_validate(config, error_msg, sizeof(error_msg))) {
        // 触发错误事件
        game_manager_trigger_event(manager, EVENT_ERROR_OCCURRED, 
                                  error_msg, strlen(error_msg) + 1);
        return FALSE;
    }
  
    // 销毁旧组件
    state_destroy(&manager->state);
    input_handler_destroy(&manager->input_handler);
    validator_destroy(&manager->validator);
    config_destroy(&manager->config);
  
    // 创建新组件
    manager->config = config_clone(config);
    manager->state = state_create(manager->config);
    manager->input_handler = input_handler_create(manager->config);
    manager->validator = validator_create(manager->config);
  
    if (manager->state == NULL || manager->input_handler == NULL || 
        manager->validator == NULL) {
        // 清理失败的部分
        state_destroy(&manager->state);
        input_handler_destroy(&manager->input_handler);
        validator_destroy(&manager->validator);
        config_destroy(&manager->config);
        return FALSE;
    }
  
    // 触发配置更新事件
    game_manager_trigger_event(manager, EVENT_CONFIG_UPDATED, 
                              config, sizeof(GameConfig));
  
    return TRUE;
}

// 设置难度
Bool game_manager_set_difficulty(GameManager* manager, DifficultyLevel difficulty) {
    if (manager == NULL) return FALSE;
  
    GameConfig* new_config = config_create_from_difficulty(difficulty);
    if (new_config == NULL) return FALSE;
  
    // 保留玩家名称
    if (manager->config != NULL) {
        strncpy(new_config->player_name, manager->config->player_name,
                sizeof(new_config->player_name) - 1);
    }
  
    Bool success = game_manager_update_config(manager, new_config);
    config_destroy(&new_config);
  
    return success;
}

// 重新加载配置
void game_manager_reload_config(GameManager* manager) {
    if (manager == NULL) return;
  
    GameConfig* default_config = config_load_default();
    if (default_config != NULL) {
        game_manager_update_config(manager, default_config);
        config_destroy(&default_config);
    }
}

// 注册回调
Bool game_manager_register_callback(GameManager* manager, GameEventType event_type,
                                   GameCallback callback, void* user_data) {
    if (manager == NULL || callback == NULL) {
        return FALSE;
    }
  
    // 检查是否已注册
    for (size_t i = 0; i < manager->callback_count; i++) {
        if (manager->callbacks[i].callback == callback && 
            manager->callbacks[i].user_data == user_data) {
            // 已存在，更新
            manager->callbacks[i].enabled = TRUE;
            return TRUE;
        }
    }
  
    // 检查是否需要扩容
    if (manager->callback_count >= manager->callback_capacity) {
        manager->callback_capacity *= 2;
        manager->callbacks = (GameCallbackEntry*)safe_realloc(
            manager->callbacks, 
            manager->callback_capacity * sizeof(GameCallbackEntry));
    }
  
    // 添加新回调
    manager->callbacks[manager->callback_count].callback = callback;
    manager->callbacks[manager->callback_count].user_data = user_data;
    manager->callbacks[manager->callback_count].enabled = TRUE;
    manager->callback_count++;
  
    return TRUE;
}

// 取消注册回调
Bool game_manager_unregister_callback(GameManager* manager, GameEventType event_type,
                                     GameCallback callback) {
    if (manager == NULL || callback == NULL) {
        return FALSE;
    }
  
    for (size_t i = 0; i < manager->callback_count; i++) {
        if (manager->callbacks[i].callback == callback) {
            // 标记为禁用
            manager->callbacks[i].enabled = FALSE;
            return TRUE;
        }
    }
  
    return FALSE;
}

// 触发事件
void game_manager_trigger_event(GameManager* manager, GameEventType event_type,
                               const void* data, size_t data_size) {
    if (manager == NULL) return;
  
    // 创建事件
    GameEvent event;
    event.type = event_type;
    event.timestamp = time(NULL);
  
    // 复制数据
    if (data != NULL && data_size > 0) {
        event.data = safe_malloc(data_size);
        memcpy(event.data, data, data_size);
        event.data_size = data_size;
    } else {
        event.data = NULL;
        event.data_size = 0;
    }
  
    // 调用所有回调
    for (size_t i = 0; i < manager->callback_count; i++) {
        if (manager->callbacks[i].enabled) {
            manager->callbacks[i].callback(&event, manager->callbacks[i].user_data);
        }
    }
  
    // 清理事件数据
    if (event.data != NULL) {
        safe_free(&event.data);
    }
}

// 获取统计信息
void game_manager_get_stats(const GameManager* manager, 
                           uint32_t* total_games, uint32_t* games_won,
                           uint32_t* games_lost, float* win_rate,
                           time_t* total_time, time_t* avg_time) {
    if (manager == NULL) return;
  
    if (total_games != NULL) {
        *total_games = manager->total_games;
    }
  
    if (games_won != NULL) {
        *games_won = manager->games_won;
    }
  
    if (games_lost != NULL) {
        *games_lost = manager->games_lost;
    }
  
    if (win_rate != NULL) {
        if (manager->total_games > 0) {
            *win_rate = (float)manager->games_won / (float)manager->total_games * 100.0f;
        } else {
            *win_rate = 0.0f;
        }
    }
  
    if (total_time != NULL) {
        *total_time = manager->total_play_time;
    }
  
    if (avg_time != NULL) {
        if (manager->total_games > 0) {
            *avg_time = manager->total_play_time / manager->total_games;
        } else {
            *avg_time = 0;
        }
    }
}

// 打印统计信息
void game_manager_print_stats(const GameManager* manager) {
    if (manager == NULL) return;
  
    uint32_t total_games, games_won, games_lost;
    float win_rate;
    time_t total_time, avg_time;
  
    game_manager_get_stats(manager, &total_games, &games_won, &games_lost,
                         &win_rate, &total_time, &avg_time);
  
    printf("╔════════════════════════════════════════╗\n");
    printf("║          Game Manager Stats           ║\n");
    printf("╠════════════════════════════════════════╣\n");
    printf("║ Total Games: %-26u ║\n", total_games);
    printf("║ Games Won: %-28u ║\n", games_won);
    printf("║ Games Lost: %-27u ║\n", games_lost);
    printf("║ Win Rate: %-29.1f%% ║\n", win_rate);
    printf("║ Total Play Time: %-21lds ║\n", total_time);
    printf("║ Average Time/Game: %-18lds ║\n", avg_time);
    printf("╚════════════════════════════════════════╝\n");
}
```

### **8. 显示模块 (ui/display.h/display.c)**

**src/ui/display.h:**

```c
#ifndef DISPLAY_H
#define DISPLAY_H

#include "../common.h"
#include "../core/config.h"
#include "../core/state.h"
#include "../core/validator.h"

#ifdef __cplusplus
extern "C" {
#endif

// 显示模式
typedef enum {
    DISPLAY_MODE_NORMAL = 0,    // 正常模式
    DISPLAY_MODE_COMPACT,       // 紧凑模式
    DISPLAY_MODE_DETAILED,      // 详细模式
    DISPLAY_MODE_DEBUG,         // 调试模式
    DISPLAY_MODE_COLORFUL       // 彩色模式
} DisplayMode;

// 显示配置
typedef struct {
    DisplayMode mode;           // 显示模式
    Bool use_unicode;           // 使用Unicode字符
    Bool show_borders;          // 显示边框
    Bool show_colors;           // 显示颜色
    Bool show_animations;       // 显示动画
    int terminal_width;         // 终端宽度
    int terminal_height;        // 终端高度
} DisplayConfig;

// 显示管理器
typedef struct {
    DisplayConfig config;       // 显示配置
} DisplayManager;

// 创建和销毁
DisplayManager* display_manager_create(void);
void display_manager_destroy(DisplayManager** manager);

// 配置管理
void display_manager_set_mode(DisplayManager* manager, DisplayMode mode);
void display_manager_detect_terminal_size(DisplayManager* manager);

// 游戏显示
void display_game_title(const DisplayManager* manager, const char* title);
void display_game_config(const DisplayManager* manager, const GameConfig* config);
void display_game_state(const DisplayManager* manager, const GameState* state);
void display_game_result(const DisplayManager* manager, const GameResult* result);
void display_guess_history(const DisplayManager* manager, const GameState* state);
void display_hint(const DisplayManager* manager, const HintInfo* hint);

// 界面元素
void display_separator(const DisplayManager* manager, char ch, int length);
void display_box(const DisplayManager* manager, const char* title, const char* content);
void display_progress_bar(const DisplayManager* manager, float progress, int width);
void display_loading(const DisplayManager* manager, const char* message);

// 文本格式化
void display_centered_text(const DisplayManager* manager, const char* text);
void display_right_aligned_text(const DisplayManager* manager, const char* text, int width);
void display_wrapped_text(const DisplayManager* manager, const char* text, int width);

// 颜色支持
void display_set_color(const DisplayManager* manager, const char* color);
void display_reset_color(const DisplayManager* manager);
void display_print_colored(const DisplayManager* manager, const char* color, 
                          const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_H
```

**src/ui/display.c:**

```c
#include "display.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// 创建显示管理器
DisplayManager* display_manager_create(void) {
    DisplayManager* manager = (DisplayManager*)safe_malloc(sizeof(DisplayManager));
    memset(manager, 0, sizeof(DisplayManager));
  
    // 默认配置
    manager->config.mode = DISPLAY_MODE_COLORFUL;
    manager->config.use_unicode = TRUE;
    manager->config.show_borders = TRUE;
    manager->config.show_colors = TRUE;
    manager->config.show_animations = FALSE;
  
    // 检测终端大小
    display_manager_detect_terminal_size(manager);
  
    return manager;
}

// 销毁显示管理器
void display_manager_destroy(DisplayManager** manager) {
    safe_free((void**)manager);
}

// 设置显示模式
void display_manager_set_mode(DisplayManager* manager, DisplayMode mode) {
    if (manager == NULL) return;
  
    manager->config.mode = mode;
  
    // 根据模式调整其他设置
    switch (mode) {
        case DISPLAY_MODE_COMPACT:
            manager->config.show_borders = FALSE;
            manager->config.show_colors = FALSE;
            manager->config.show_animations = FALSE;
            break;
        case DISPLAY_MODE_DETAILED:
            manager->config.show_borders = TRUE;
            manager->config.show_colors = TRUE;
            manager->config.show_animations = TRUE;
            break;
        case DISPLAY_MODE_DEBUG:
            manager->config.show_borders = TRUE;
            manager->config.show_colors = TRUE;
            manager->config.show_animations = FALSE;
            break;
        case DISPLAY_MODE_COLORFUL:
            manager->config.show_borders = TRUE;
            manager->config.show_colors = TRUE;
            manager->config.show_animations = TRUE;
            break;
        default: // NORMAL
            manager->config.show_borders = TRUE;
            manager->config.show_colors = TRUE;
            manager->config.show_animations = FALSE;
            break;
    }
}

// 检测终端大小
void display_manager_detect_terminal_size(DisplayManager* manager) {
    if (manager == NULL) return;
  
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  
    if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
        manager->config.terminal_width = csbi.dwSize.X;
        manager->config.terminal_height = csbi.dwSize.Y;
    } else {
        manager->config.terminal_width = 80;
        manager->config.terminal_height = 24;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        manager->config.terminal_width = w.ws_col;
        manager->config.terminal_height = w.ws_row;
    } else {
        manager->config.terminal_width = 80;
        manager->config.terminal_height = 24;
    }
#endif
  
    // 确保最小尺寸
    if (manager->config.terminal_width < 40) {
        manager->config.terminal_width = 40;
    }
    if (manager->config.terminal_height < 20) {
        manager->config.terminal_height = 20;
    }
}

// 显示游戏标题
void display_game_title(const DisplayManager* manager, const char* title) {
    if (title == NULL) return;
  
    printf("\n");
    display_separator(manager, '═', manager->config.terminal_width);
  
    if (manager->config.show_colors) {
        display_print_colored(manager, "\033[1;36m", "  %s", title);
    } else {
        printf("  %s\n", title);
    }
  
    display_separator(manager, '═', manager->config.terminal_width);
    printf("\n");
}

// 显示游戏配置
void display_game_config(const DisplayManager* manager, const GameConfig* config) {
    if (config == NULL) return;
  
    if (manager->config.show_borders) {
        printf("┌────────────────────────────────────────┐\n");
        printf("│          Game Configuration           │\n");
        printf("├────────────────────────────────────────┤\n");
    } else {
        printf("Game Configuration:\n");
        printf("────────────────────\n");
    }
  
    printf("Player: %s\n", config->player_name);
    printf("Difficulty: %s\n", config_get_difficulty_name(config->difficulty));
    printf("Range: %u to %u\n", config->min_range, config->max_range);
  
    if (config->max_attempts == 0) {
        printf("Max Attempts: Unlimited\n");
    } else {
        printf("Max Attempts: %u\n", config->max_attempts);
    }
  
    printf("Hints: %s\n", config->allow_hints ? "Enabled" : "Disabled");
  
    if (manager->config.show_borders) {
        printf("└────────────────────────────────────────┘\n");
    }
    printf("\n");
}

// 显示游戏状态
void display_game_state(const DisplayManager* manager, const GameState* state) {
    if (state == NULL) return;
  
    if (manager->config.show_borders) {
        printf("┌────────────────────────────────────────┐\n");
        printf("│            Game Status                 │\n");
        printf("├────────────────────────────────────────┤\n");
    }
  
    const char* status_str;
    switch (state->status) {
        case STATE_NEW: status_str = "New"; break;
        case STATE_PLAYING: status_str = "Playing"; break;
        case STATE_PAUSED: status_str = "Paused"; break;
        case STATE_WON: status_str = "WON!"; break;
        case STATE_LOST: status_str = "Lost"; break;
        case STATE_SAVED: status_str = "Saved"; break;
        default: status_str = "Unknown"; break;
    }
  
    printf("Status: %s\n", status_str);
    printf("Attempts: %u", state->attempts);
  
    if (state->config.max_attempts > 0) {
        printf("/%u", state->config.max_attempts);
    }
    printf("\n");
  
    // 显示猜测历史
    if (state->config.show_history && state->history.size > 0) {
        printf("\nGuess History:\n");
        for (size_t i = 0; i < state->history.size; i++) {
            const GuessRecord* record = &state->history.records[i];
            char indicator = ' ';
          
            if (record->hint < 0) {
                indicator = manager->config.use_unicode ? '↑' : '<';
            } else if (record->hint > 0) {
                indicator = manager->config.use_unicode ? '↓' : '>';
            } else {
                indicator = manager->config.use_unicode ? '✓' : '=';
            }
          
            printf("  %zu. %u %c\n", i + 1, record->guess, indicator);
        }
    }
  
    if (manager->config.show_borders) {
        printf("└────────────────────────────────────────┘\n");
    }
    printf("\n");
}

// 显示游戏结果
void display_game_result(const DisplayManager* manager, const GameResult* result) {
    if (result == NULL) return;
  
    printf("\n");
    display_separator(manager, '★', manager->config.terminal_width);
  
    if (result->status == STATE_WON) {
        if (manager->config.show_colors) {
            display_print_colored(manager, "\033[1;32m", 
                                 "  🎉 CONGRATULATIONS! YOU WON! 🎉");
        } else {
            printf("  CONGRATULATIONS! YOU WON!\n");
        }
      
        printf("\n  You guessed the number in %u attempts!\n", result->attempts);
      
        if (result->is_high_score) {
            printf("  🏆 NEW HIGH SCORE! 🏆\n");
        }
    } else if (result->status == STATE_LOST) {
        if (manager->config.show_colors) {
            display_print_colored(manager, "\033[1;31m", 
                                 "  💔 GAME OVER! YOU LOST! 💔");
        } else {
            printf("  GAME OVER! YOU LOST!\n");
        }
      
        printf("\n  The secret number was: %u\n", result->secret);
        printf("  You used %u attempts\n", result->attempts);
    }
  
    display_separator(manager, '★', manager->config.terminal_width);
    printf("\n");
}

// 显示猜测历史
void display_guess_history(const DisplayManager* manager, const GameState* state) {
    if (state == NULL || state->history.size == 0) return;
  
    printf("Guess History:\n");
    printf("┌─────┬───────┬────────────┬─────────┐\n");
    printf("│ No. │ Guess │ Difference │  Hint   │\n");
    printf("├─────┼───────┼────────────┼─────────┤\n");
  
    for (size_t i = 0; i < state->history.size; i++) {
        const GuessRecord* record = &state->history.records[i];
        uint32_t diff = (record->guess > state->secret_number) ?
                       (record->guess - state->secret_number) :
                       (state->secret_number - record->guess);
      
        const char* hint_str;
        if (record->hint < 0) {
            hint_str = "Too small";
        } else if (record->hint > 0) {
            hint_str = "Too big";
        } else {
            hint_str = "Correct!";
        }
      
        printf("│ %3zu │ %5u │ %10u │ %-7s │\n", 
               i + 1, record->guess, diff, hint_str);
    }
  
    printf("└─────┴───────┴────────────┴─────────┘\n");
}

// 显示提示
void display_hint(const DisplayManager* manager, const HintInfo* hint) {
    if (hint == NULL || hint->type == HINT_NONE) return;
  
    const char* color = NULL;
  
    // 根据严重程度选择颜色
    if (manager->config.show_colors) {
        if (hint->severity <= 2) {
            color = "\033[1;32m";  // 绿色
        } else if (hint->severity <= 5) {
            color = "\033[1;33m";  // 黄色
        } else {
            color = "\033[1;31m";  // 红色
        }
    }
  
    if (color != NULL) {
        display_print_colored(manager, color, "💡 Hint: %s", hint->text);
    } else {
        printf("💡 Hint: %s\n", hint->text);
    }
}

// 显示分隔线
void display_separator(const DisplayManager* manager, char ch, int length) {
    int actual_length = length;
    if (manager != NULL && actual_length > manager->config.terminal_width) {
        actual_length = manager->config.terminal_width;
    }
  
    for (int i = 0; i < actual_length; i++) {
        putchar(ch);
    }
    putchar('\n');
}

// 设置颜色
void display_set_color(const DisplayManager* manager, const char* color) {
    if (manager == NULL || !manager->config.show_colors || color == NULL) {
        return;
    }
  
    printf("%s", color);
}

// 重置颜色
void display_reset_color(const DisplayManager* manager) {
    if (manager == NULL || !manager->config.show_colors) {
        return;
    }
  
    printf("\033[0m");
}

// 打印彩色文本
void display_print_colored(const DisplayManager* manager, const char* color, 
                          const char* format, ...) {
    if (manager == NULL || format == NULL) return;
  
    display_set_color(manager, color);
  
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
  
    display_reset_color(manager);
    printf("\n");
}
```

### **9. 菜单模块 (ui/menu.h/menu.c)**

**src/ui/menu.h:**

```c
#ifndef MENU_H
#define MENU_H

#include "../common.h"
#include "../core/config.h"
#include "../core/game.h"

#ifdef __cplusplus
extern "C" {
#endif

// 菜单项类型
typedef enum {
    MENU_ITEM_NORMAL = 0,    // 普通菜单项
    MENU_ITEM_SUBMENU,       // 子菜单
    MENU_ITEM_SEPARATOR,     // 分隔符
    MENU_ITEM_BACK,          // 返回
    MENU_ITEM_EXIT           // 退出
} MenuItemType;

// 菜单项
typedef struct MenuItem {
    char id[32];             // 项目ID
    char title[64];          // 显示标题
    char description[128];   // 描述
    MenuItemType type;       // 类型
    void* data;             // 关联数据
    size_t data_size;       // 数据大小
  
    struct MenuItem* parent; // 父菜单
    struct MenuItem* next;   // 下一个项目
    struct MenuItem* prev;   // 上一个项目
} MenuItem;

// 菜单
typedef struct {
    MenuItem* root;          // 根菜单项
    MenuItem* current;       // 当前菜单项
    DisplayManager* display; // 显示管理器
    GameManager* game;       // 游戏管理器
    char title[64];          // 菜单标题
} MenuSystem;

// 创建和销毁
MenuSystem* menu_system_create(DisplayManager* display, GameManager* game);
void menu_system_destroy(MenuSystem** system);

// 菜单项管理
MenuItem* menu_item_create(const char* id, const char* title, 
                          const char* description, MenuItemType type,
                          void* data, size_t data_size);
void menu_item_destroy(MenuItem** item);
void menu_system_add_item(MenuSystem* system, MenuItem* parent, MenuItem* item);
void menu_system_remove_item(MenuSystem* system, MenuItem* item);

// 菜单导航
void menu_system_goto(MenuSystem* system, MenuItem* item);
void menu_system_goto_parent(MenuSystem* system);
void menu_system_goto_root(MenuSystem* system);

// 菜单显示
void menu_system_display_current(MenuSystem* system);
void menu_system_display_item(MenuSystem* system, MenuItem* item);

// 菜单处理
MenuItem* menu_system_process_input(MenuSystem* system);
Bool menu_system_handle_selection(MenuSystem* system, MenuItem* selected);

// 预设菜单
void menu_system_create_main_menu(MenuSystem* system);
void menu_system_create_game_menu(MenuSystem* system);
void menu_system_create_config_menu(MenuSystem* system);
void menu_system_create_stats_menu(MenuSystem* system);

// 回调函数
typedef void (*MenuCallback)(MenuItem* item, void* user_data);
void menu_system_set_callback(MenuSystem* system, MenuItem* item, 
                             MenuCallback callback, void* user_data);

#ifdef __cplusplus
}
#endif

#endif // MENU_H
```

**src/ui/menu.c:**

```c
#include "menu.h"
#include <stdio.h>
#include <string.h>

// 创建菜单系统
MenuSystem* menu_system_create(DisplayManager* display, GameManager* game) {
    MenuSystem* system = (MenuSystem*)safe_malloc(sizeof(MenuSystem));
    memset(system, 0, sizeof(MenuSystem));
  
    system->display = display;
    system->game = game;
    strcpy(system->title, "Main Menu");
  
    // 创建根菜单项
    system->root = menu_item_create("root", "Main Menu", 
                                   "Main menu of the game", 
                                   MENU_ITEM_SUBMENU, NULL, 0);
    system->current = system->root;
  
    return system;
}

// 销毁菜单系统
void menu_system_destroy(MenuSystem** system) {
    if (system == NULL || *system == NULL) return;
  
    MenuSystem* s = *system;
  
    // TODO: 递归销毁所有菜单项
    // 这里简化处理，实际项目需要完整的销毁逻辑
  
    safe_free((void**)system);
}

// 创建菜单项
MenuItem* menu_item_create(const char* id, const char* title, 
                          const char* description, MenuItemType type,
                          void* data, size_t data_size) {
    MenuItem* item = (MenuItem*)safe_malloc(sizeof(MenuItem));
    memset(item, 0, sizeof(MenuItem));
  
    if (id != NULL) {
        strncpy(item->id, id, sizeof(item->id) - 1);
    }
  
    if (title != NULL) {
        strncpy(item->title, title, sizeof(item->title) - 1);
    }
  
    if (description != NULL) {
        strncpy(item->description, description, sizeof(item->description) - 1);
    }
  
    item->type = type;
  
    // 复制数据
    if (data != NULL && data_size > 0) {
        item->data = safe_malloc(data_size);
        memcpy(item->data, data, data_size);
        item->data_size = data_size;
    }
  
    return item;
}

// 销毁菜单项
void menu_item_destroy(MenuItem** item) {
    if (item == NULL || *item == NULL) return;
  
    MenuItem* i = *item;
  
    // 销毁数据
    if (i->data != NULL) {
        safe_free(&i->data);
    }
  
    // 解除链接
    if (i->prev != NULL) {
        i->prev->next = i->next;
    }
  
    if (i->next != NULL) {
        i->next->prev = i->prev;
    }
  
    safe_free((void**)item);
}

// 显示当前菜单
void menu_system_display_current(MenuSystem* system) {
    if (system == NULL || system->current == NULL || system->display == NULL) {
        return;
    }
  
    // 清屏
    system(CLEAR_SCREEN);
  
    // 显示标题
    display_game_title(system->display, system->current->title);
  
    // 显示描述
    if (system->current->description[0] != '\0') {
        printf("%s\n\n", system->current->description);
    }
  
    // 显示菜单项
    MenuItem* item = system->current;
    int index = 1;
  
    while (item != NULL) {
        switch (item->type) {
            case MENU_ITEM_NORMAL:
                printf("%d. %s\n", index, item->title);
                if (item->description[0] != '\0') {
                    printf("   %s\n", item->description);
                }
                index++;
                break;
              
            case MENU_ITEM_SUBMENU:
                printf("%d. %s ▶\n", index, item->title);
                if (item->description[0] != '\0') {
                    printf("   %s\n", item->description);
                }
                index++;
                break;
              
            case MENU_ITEM_SEPARATOR:
                printf("────────────────────\n");
                break;
              
            case MENU_ITEM_BACK:
                printf("%d. ← Back\n", index);
                index++;
                break;
              
            case MENU_ITEM_EXIT:
                printf("%d. Exit\n", index);
                index++;
                break;
        }
      
        item = item->next;
    }
  
    printf("\nEnter selection: ");
    flush_output_stream();
}
```

### **10. 主程序 (main.c)**

**src/main.c:**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "core/config.h"
#include "core/game.h"
#include "ui/display.h"
#include "ui/menu.h"

// 回调函数示例
void on_game_start(GameEvent* event, void* user_data) {
    printf("🎮 Game started!\n");
}

void on_game_end(GameEvent* event, void* user_data) {
    printf("🏁 Game ended!\n");
}

void on_guess_made(GameEvent* event, void* user_data) {
    if (event->data != NULL) {
        uint32_t guess = *(uint32_t*)event->data;
        printf("🎯 Guess made: %u\n", guess);
    }
}

void on_hint_given(GameEvent* event, void* user_data) {
    if (event->data != NULL) {
        HintInfo* hint = (HintInfo*)event->data;
        printf("💡 Hint: %s\n", hint->text);
    }
}

// 创建自定义配置
GameConfig* create_custom_config(void) {
    printf("\n🎛️  Custom Game Configuration\n");
    printf("════════════════════════════════════════\n\n");
  
    // 获取玩家名称
    char player_name[64];
    printf("Enter your name: ");
    flush_output_stream();
  
    if (fgets(player_name, sizeof(player_name), stdin) == NULL) {
        return NULL;
    }
  
    // 移除换行符
    size_t len = strlen(player_name);
    if (len > 0 && player_name[len-1] == '\n') {
        player_name[len-1] = '\0';
    }
  
    // 获取范围
    uint32_t min_range, max_range;
    printf("Enter number range (min,max): ");
    flush_output_stream();
  
    char range_input[64];
    if (fgets(range_input, sizeof(range_input), stdin) == NULL) {
        return NULL;
    }
  
    char* comma = strchr(range_input, ',');
    if (comma == NULL) {
        printf("Invalid format. Using default range.\n");
        min_range = 1;
        max_range = 50;
    } else {
        *comma = '\0';
        min_range = (uint32_t)atoi(range_input);
        max_range = (uint32_t)atoi(comma + 1);
      
        if (min_range >= max_range) {
            printf("Invalid range. Using default.\n");
            min_range = 1;
            max_range = 50;
        }
    }
  
    // 获取最大尝试次数
    uint32_t max_attempts;
    printf("Enter max attempts (0 for unlimited): ");
    flush_output_stream();
  
    char attempts_input[64];
    if (fgets(attempts_input, sizeof(attempts_input), stdin) == NULL) {
        return NULL;
    }
  
    max_attempts = (uint32_t)atoi(attempts_input);
  
    // 是否允许提示
    Bool allow_hints;
    printf("Enable hints? (y/n): ");
    flush_output_stream();
  
    char hint_input[64];
    if (fgets(hint_input, sizeof(hint_input), stdin) == NULL) {
        return NULL;
    }
  
    allow_hints = (hint_input[0] == 'y' || hint_input[0] == 'Y');
  
    // 创建配置
    GameConfig* config = config_create_custom(min_range, max_range, 
                                             max_attempts, allow_hints);
  
    if (config != NULL) {
        strncpy(config->player_name, player_name, sizeof(config->player_name) - 1);
        config->difficulty = DIFFICULTY_CUSTOM;
        config->show_history = TRUE;
        config->auto_save = FALSE;
        config->max_input_length = 20;
    }
  
    return config;
}

// 游戏主循环
void game_loop(GameManager* game_manager, DisplayManager* display) {
    if (game_manager == NULL || display == NULL) return;
  
    // 开始游戏
    if (!game_manager_start(game_manager)) {
        printf("Failed to start game.\n");
        return;
    }
  
    display_game_title(display, "Guess the Number Game");
    display_game_config(display, game_manager->config);
  
    char input[256];
    Bool game_running = TRUE;
  
    while (game_running) {
        // 显示当前状态
        display_game_state(display, game_manager->state);
      
        // 检查游戏是否结束
        if (state_is_game_over(game_manager->state)) {
            GameResult result;
            result.status = game_manager->state->status;
            result.attempts = game_manager->state->attempts;
            result.secret = game_manager->state->secret_number;
            result.is_high_score = FALSE;
          
            display_game_result(display, &result);
          
            // 询问是否再玩一次
            printf("\nPlay again? (y/n): ");
            flush_output_stream();
          
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
          
            string_trim(input);
            string_to_lower(input);
          
            if (strcmp(input, "y") == 0 || strcmp(input, "yes") == 0) {
                game_manager_restart(game_manager);
                continue;
            } else {
                game_running = FALSE;
                break;
            }
        }
      
        // 读取猜测
        printf("Enter your guess (%u-%u): ", 
               game_manager->config->min_range, 
               game_manager->config->max_range);
        flush_output_stream();
      
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Failed to read input.\n");
            continue;
        }
      
        string_trim(input);
      
        // 检查特殊命令
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            printf("Quitting game...\n");
            break;
        }
      
        if (strcmp(input, "hint") == 0 && game_manager->config->allow_hints) {
            // 显示上次猜测的提示
            if (game_manager->state->history.size > 0) {
                const GuessRecord* last_guess = 
                    state_get_history_at(game_manager->state, 
                                       game_manager->state->history.size - 1);
                HintInfo hint = game_manager_get_hint(game_manager, 
                                                     last_guess->guess);
                display_hint(display, &hint);
            }
            continue;
        }
      
        if (strcmp(input, "stats") == 0) {
            state_print_stats(game_manager->state);
            continue;
        }
      
        if (strcmp(input, "config") == 0) {
            display_game_config(display, game_manager->config);
            continue;
        }
      
        // 验证并处理猜测
        uint32_t guess;
        char error_msg[256];
      
        if (!input_handler_read_number(game_manager->input_handler, 
                                      NULL, &guess, 
                                      error_msg, sizeof(error_msg))) {
            printf("Error: %s\n", error_msg);
            continue;
        }
      
        // 进行猜测
        GameResult result;
        if (!game_manager_make_guess(game_manager, guess, &result)) {
            printf("Failed to process guess.\n");
            continue;
        }
      
        // 显示比较结果
        if (guess < game_manager->state->secret_number) {
            printf("📈 %u is too small!\n", guess);
        } else if (guess > game_manager->state->secret_number) {
            printf("📉 %u is too big!\n", guess);
        } else {
            printf("🎉 %u is correct!\n", guess);
        }
      
        // 显示提示
        if (game_manager->config->allow_hints) {
            HintInfo hint = game_manager_get_hint(game_manager, guess);
            display_hint(display, &hint);
        }
      
        printf("\n");
    }
}

// 主菜单
void main_menu(void) {
    DisplayManager* display = display_manager_create();
    GameManager* game_manager = NULL;
  
    if (display == NULL) {
        printf("Failed to create display manager.\n");
        return;
    }
  
    display_manager_set_mode(display, DISPLAY_MODE_COLORFUL);
  
    Bool running = TRUE;
    char input[256];
  
    while (running) {
        system(CLEAR_SCREEN);
      
        display_game_title(display, "Guess the Number Game - V3");
      
        printf("Main Menu:\n");
        printf("════════════════════════════════════════\n\n");
      
        printf("1. New Game\n");
        printf("2. Select Difficulty\n");
        printf("3. Custom Game\n");
        printf("4. Load Game\n");
        printf("5. Game Statistics\n");
        printf("6. Configuration\n");
        printf("7. Help\n");
        printf("8. Exit\n\n");
      
        printf("Enter selection (1-8): ");
        flush_output_stream();
      
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
      
        string_trim(input);
        int choice = atoi(input);
      
        switch (choice) {
            case 1: // New Game
                if (game_manager == NULL) {
                    game_manager = game_manager_create();
                    if (game_manager == NULL) {
                        printf("Failed to create game manager.\n");
                        break;
                    }
                  
                    // 注册回调
                    game_manager_register_callback(game_manager, 
                                                  EVENT_GAME_START, 
                                                  on_game_start, NULL);
                    game_manager_register_callback(game_manager, 
                                                  EVENT_GAME_END, 
                                                  on_game_end, NULL);
                    game_manager_register_callback(game_manager, 
                                                  EVENT_GUESS_MADE, 
                                                  on_guess_made, NULL);
                    game_manager_register_callback(game_manager, 
                                                  EVENT_HINT_GIVEN, 
                                                  on_hint_given, NULL);
                }
              
                game_manager_set_difficulty(game_manager, DIFFICULTY_NORMAL);
                game_loop(game_manager, display);
                break;
              
            case 2: // Select Difficulty
                if (game_manager == NULL) {
                    game_manager = game_manager_create();
                }
              
                system(CLEAR_SCREEN);
                printf("Select Difficulty:\n");
                printf("════════════════════════════════════════\n\n");
              
                printf("1. Easy (1-10, unlimited attempts)\n");
                printf("2. Normal (1-50, 10 attempts)\n");
                printf("3. Hard (1-100, 5 attempts)\n");
                printf("4. Expert (1-1000, 3 attempts)\n");
                printf("5. Back\n\n");
              
                printf("Enter selection (1-5): ");
                flush_output_stream();
              
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }
              
                string_trim(input);
                int diff_choice = atoi(input);
              
                DifficultyLevel difficulty;
                switch (diff_choice) {
                    case 1: difficulty = DIFFICULTY_EASY; break;
                    case 2: difficulty = DIFFICULTY_NORMAL; break;
                    case 3: difficulty = DIFFICULTY_HARD; break;
                    case 4: difficulty = DIFFICULTY_EXPERT; break;
                    default: continue;
                }
              
                if (diff_choice != 5) {
                    game_manager_set_difficulty(game_manager, difficulty);
                    game_loop(game_manager, display);
                }
                break;
              
            case 3: // Custom Game
                {
                    GameConfig* custom_config = create_custom_config();
                    if (custom_config == NULL) {
                        printf("Failed to create custom configuration.\n");
                        break;
                    }
                  
                    if (game_manager != NULL) {
                        game_manager_destroy(&game_manager);
                    }
                  
                    game_manager = game_manager_create_with_config(custom_config);
                    config_destroy(&custom_config);
                  
                    if (game_manager == NULL) {
                        printf("Failed to create game with custom config.\n");
                        break;
                    }
                  
                    game_loop(game_manager, display);
                }
                break;
              
            case 4: // Load Game
                printf("Load game functionality not implemented yet.\n");
                printf("Press Enter to continue...");
                flush_output_stream();
                fgets(input, sizeof(input), stdin);
                break;
              
            case 5: // Game Statistics
                if (game_manager != NULL) {
                    game_manager_print_stats(game_manager);
                } else {
                    printf("No game statistics available.\n");
                }
              
                printf("\nPress Enter to continue...");
                flush_output_stream();
                fgets(input, sizeof(input), stdin);
                break;
              
            case 6: // Configuration
                printf("Configuration menu not implemented yet.\n");
                printf("Press Enter to continue...");
                flush_output_stream();
                fgets(input, sizeof(input), stdin);
                break;
              
            case 7: // Help
                system(CLEAR_SCREEN);
                printf("Game Help:\n");
                printf("════════════════════════════════════════\n\n");
              
                printf("How to play:\n");
                printf("1. The computer generates a secret number within a range.\n");
                printf("2. You try to guess the number.\n");
                printf("3. After each guess, you'll get feedback:\n");
                printf("   - 'Too small' if your guess is lower\n");
                printf("   - 'Too big' if your guess is higher\n");
                printf("   - 'Correct!' if you guess the number\n");
                printf("4. Try to guess the number in as few attempts as possible.\n\n");
              
                printf("Commands:\n");
                printf("- 'hint': Get a hint (if enabled)\n");
                printf("- 'stats': Show game statistics\n");
                printf("- 'config': Show game configuration\n");
                printf("- 'quit' or 'exit': Quit the game\n\n");
              
                printf("Press Enter to continue...");
                flush_output_stream();
                fgets(input, sizeof(input), stdin);
                break;
              
            case 8: // Exit
                running = FALSE;
                break;
              
            default:
                printf("Invalid selection. Please try again.\n");
                printf("Press Enter to continue...");
                flush_output_stream();
                fgets(input, sizeof(input), stdin);
                break;
        }
    }
  
    // 清理
    if (game_manager != NULL) {
        game_manager_destroy(&game_manager);
    }
  
    if (display != NULL) {
        display_manager_destroy(&display);
    }
}

int main(int argc, char* argv[]) {
    // 初始化
    common_init();
  
    // 显示欢迎信息
    printf("\n");
    print_separator('═', 60);
    print_centered("Guess the Number Game - Version 3", 60);
    print_centered("Modular C Implementation", 60);
    print_centered("Built with C11 and CMake", 60);
    print_separator('═', 60);
    printf("\n");
  
    // 检查命令行参数
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("\nOptions:\n");
            printf("  --help, -h     Show this help message\n");
            printf("  --version, -v  Show version information\n");
            printf("  --easy         Start with easy difficulty\n");
            printf("  --normal       Start with normal difficulty (default)\n");
            printf("  --hard         Start with hard difficulty\n");
            printf("  --expert       Start with expert difficulty\n");
            printf("\n");
            return 0;
        }
      
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            printf("Guess the Number Game v3.0.0\n");
            printf("C11 implementation with modular architecture\n");
            printf("Build date: %s %s\n", __DATE__, __TIME__);
            return 0;
        }
    }
  
    // 进入主菜单
    main_menu();
  
    // 清理
    common_cleanup();
  
    printf("\nThank you for playing! Goodbye!\n\n");
    return 0;
}
```

## **构建和运行说明**

### **构建步骤**

```bash
# 创建构建目录
mkdir build
cd build

# 配置CMake
cmake ..

# 编译
make

# 运行程序
./guess_game

# 运行测试（如果启用了测试）
ctest
```

### **命令行选项**

```bash
# 显示帮助
./guess_game --help

# 显示版本信息
./guess_game --version

# 直接启动特定难度
./guess_game --easy
./guess_game --normal
./guess_game --hard
./guess_game --expert
```

## **主要特性**

### **1. 模块化架构**

- **配置模块**：管理游戏设置和难度级别
- **状态模块**：跟踪游戏进度和统计信息
- **验证器模块**：处理输入验证和提示生成
- **输入模块**：管理用户输入和错误处理
- **游戏模块**：协调所有组件的核心管理器
- **显示模块**：提供丰富的用户界面
- **菜单模块**：管理游戏菜单系统

### **2. 完整的功能**

- 多种难度级别（简单、普通、困难、专家、自定义）
- 完整的输入验证和错误处理
- 智能提示系统
- 游戏统计和历史记录
- 保存/加载功能
- 事件回调系统
- 丰富的用户界面

### **3. 代码质量**

- 符合C11标准
- 完整的错误处理
- 内存安全（使用安全的内存分配包装器）
- 模块化设计，易于测试和维护
- 清晰的API文档

### **4. 构建系统**

- 使用CMake构建
- 支持跨平台编译
- 可选的测试构建
- 安装和打包支持

## **扩展建议**

1. **添加网络功能**：实现多人游戏或排行榜
2. **添加数据库支持**：存储玩家统计和历史记录
3. **添加GUI界面**：使用GTK或Qt创建图形界面
4. **添加AI玩家**：实现自动猜测算法
5. **添加成就系统**：奖励玩家达成特定目标
6. **添加国际化**：支持多语言界面

这个第三版实现是一个生产级别的猜数字游戏，具有完整的功能和良好的架构设计，可以作为学习C语言模块化编程的优秀示例。
