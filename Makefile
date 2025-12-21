# 定义编译器
CC = gcc

# 定义目录
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = install/build
BIN_DIR = install/bin

# 定义源文件和目标文件
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))
TARGET = $(BIN_DIR)/main

# 编译选项
CFLAGS = -I$(INCLUDE_DIR) -Wall -std=c11 -O2

# 默认目标
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJ_FILES) | $(BIN_DIR)
        @echo "链接目标文件生成可执行文件: $@"
        $(CC) $(OBJ_FILES) -o $@
        @echo "可执行文件已生成: $@"

# 生成目标文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
        @echo "编译源文件: $<"
        $(CC) $(CFLAGS) -c $< -o $@
        @echo "目标文件已生成: $@"

# 创建build目录
$(BUILD_DIR):
        @echo "创建build目录"
        mkdir -p $(BUILD_DIR)

# 创建bin目录
$(BIN_DIR):
        @echo "创建bin目录"
        mkdir -p $(BIN_DIR)

# 清理生成文件
clean:
        @echo "清理生成文件"
        rm -rf $(BUILD_DIR) $(BIN_DIR)

# 打印帮助信息
help:
        @echo "使用方法:"
        @echo "  make        - 编译所有源文件并生成可执行文件"
        @echo "  make clean  - 清理生成文件"
        @echo "  make help   - 打印此帮助信息"

# 声明伪目标，避免与实际文件名冲突
.PHONY: all clean help