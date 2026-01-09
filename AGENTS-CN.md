# AGENTS-CN.md - Tour C++ 项目指南（中文版）

本文档为在 Tour C++ 项目中工作的 AI 代理提供指南，包括构建命令、测试流程和代码风格规范。

## 项目概述

这是一个 C/C++ 学习项目，包含模块化示例和库。项目使用 CMake 作为主要构建系统，遵循现代 C++17 标准。

## 构建命令

### CMake 构建（推荐）
```bash
# 从项目根目录配置和构建
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)

# 或使用便捷脚本
./scripts/build.sh
```

### Makefile 构建（传统）
```bash
# 简单 C 项目构建
make

# 清理构建产物
make clean
```

### 构建特定组件
```bash
# 仅构建计算器示例
cd programs/calculator_qt
mkdir -p build_cmake && cd build_cmake
cmake .. && make

# 构建猜数字游戏
cd programs/guessing_game/guessing_game_cpp
mkdir -p build_cmake && cd build_cmake
cmake .. && make
```

## Lint 和格式化命令

### 使用 clang-format 进行代码格式化
```bash
# 格式化单个文件
clang-format -i path/to/file.cpp

# 格式化项目中所有 C/C++ 文件
find . -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" | xargs clang-format -i

# 检查格式化而不应用更改
clang-format --dry-run --Werror path/to/file.cpp
```

### 使用 clang-tidy 进行静态分析
```bash
# 在文件上运行 clang-tidy
clang-tidy path/to/file.cpp --checks="*" --warnings-as-errors="bugprone-*,clang-analyzer-*"

# 首先生成 compile_commands.json
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

# 然后使用编译数据库运行 clang-tidy
run-clang-tidy -p build/
```

## 测试命令

### 运行所有测试
```bash
# 针对计算器项目
cd programs/calculator_qt/tests
./run_all_tests.sh

# 运行特定类型的测试
./run_all_tests.sh --functional-only
./run_all_tests.sh --performance-only
./run_all_tests.sh --security-only
```

### 单个测试组件
```bash
# 功能测试
cd programs/calculator_qt/tests/functional
./functional_test.sh

# 性能测试
cd programs/calculator_qt/tests/performance
./performance_test.sh

# 内存/Valgrind 测试（如果安装了 valgrind）
./valgrind_test.sh
```

### 构建测试可执行文件
```bash
# 构建并运行 C 版本
cd programs/calculator_qt/calculator_c/build_cmake
./scientific_calculator_c

# 构建并运行 C++ 版本
cd programs/calculator_qt/calculator_cpp/build_cmake
./scientific_calculator_cpp
```

## 代码风格指南

### 格式化规则（来自 .clang-format）
- **缩进**：4个空格，不使用制表符
- **大括号风格**：Stroustrup（Allman 变体）
- **指针对齐**：`int* ptr`（左对齐）
- **列限制**：80个字符
- **包含排序**：是，系统头文件优先
- **命名空间包装**：命名空间后不换行

### 命名约定
- **类**：`CamelCase`（例如：`GuessingGameV2`）
- **结构体**：`snake_case`（例如：`game_state`）
- **变量**：`camelBack`（例如：`secretNumber`）
- **常量**：`UPPER_SNAKE_CASE`（例如：`MAX_ATTEMPTS`）
- **函数**：`snake_case`（例如：`display_welcome()`）
- **私有成员**：无前缀，但使用 `private:` 部分

### 类型使用
- **基本类型**：使用标准类型（`int32_t`，`size_t` 用于大小）
- **智能指针**：优先使用 `std::unique_ptr` 而非原始指针
- **字符串**：使用 `std::string`（C++）或带长度的 `const char*`（C）
- **容器**：标准库容器（`std::vector`，`std::map`）

### 包含顺序
1. 系统头文件（`#include <iostream>`）
2. 标准库头文件（`#include <vector>`）
3. 第三方库头文件
4. 项目头文件（`#include "colorfmt.h"`）

示例：
```cpp
#include <iostream>
#include <vector>
#include <string>

#include "thirdparty/some_lib.h"

#include "project/header.h"
```

### 错误处理
- **C++**：对异常情况使用异常，对可选值使用 `std::optional`
- **C**：返回错误代码，对系统错误使用 `errno`
- **断言**：在调试构建中使用 `assert()`，不用于运行时错误处理
- **日志记录**：使用提供的 `colorfmt` 库进行彩色输出

### 头文件指南
```cpp
// 使用 #pragma once（非包含保护）
#pragma once
#ifndef CLASS_NAME_HPP  // 可选向后兼容性
#define CLASS_NAME_HPP

#include <necessary_headers>

namespace ProjectName {

class ClassName {
public:
    ClassName();
    ~ClassName() = default;

    // 公共 API
    void public_method();

private:
    // 私有实现
    void private_method_();
    int member_variable_;
};

} // namespace ProjectName

#endif // CLASS_NAME_HPP
```

### C++ 特定指南
- **C++ 版本**：最低 C++17
- **Auto 使用**：当类型从上下文中明显时使用 `auto`
- **Const 正确性**：当方法不修改状态时标记为 `const`
- **引用**：对只读参数使用 `const&`，对输出参数使用 `&`
- **移动语义**：在适当时实现移动构造函数/运算符

### C 特定指南
- **C 版本**：最低 C11
- **结构体使用**：始终使用 `typedef struct` 以获得更清晰的语法
- **内存管理**：始终在同一作用域内配对 `malloc()` 和 `free()`
- **字符串处理**：使用 `strn` 函数而非 `str` 函数（`strncpy`，`strncat`）

## 项目结构说明

- **源代码**：位于 `programs/` 和 `library/` 目录
- **头文件**：`include/` 目录包含公共头文件
- **第三方库**：`thirdparty/`
- **测试**：根目录下的 `tests/` 目录，项目特定测试在 `programs/*/tests/`
- **文档**：`docs/` 目录和整个项目中的 `*.md` 文件

## 代理的重要文件

- `.clang-format` - 格式化配置
- `.clang-tidy` - 静态分析配置
- `CMakeLists.txt` - 构建配置
- `Makefile` - 传统构建脚本
- `programs/calculator_qt/docs/calculator_architecture.md` - 示例架构文档

## 代理的常见任务

1. **添加新的示例程序**：在 `programs/` 中创建目录，包含 C 和 C++ 版本
2. **添加实用库**：在 `library/` 中创建目录，包含 CMakeLists.txt
3. **编写测试**：遵循 `programs/calculator_qt/tests/` 中的现有模式
4. **文档**：更新相关的 `.md` 文件，并在需要时添加架构图

## 环境要求

- **编译器**：支持 C++17 的 GCC/Clang
- **CMake**：版本 3.22 或更高
- **工具**：clang-format、clang-tidy、valgrind（可选）
- **系统**：Linux/macOS（Windows 使用 WSL 或 MinGW）

## 提交前的质量检查

1. 在更改的文件上运行 `clang-format`
2. 使用 CMake 构建所有受影响的组件
3. 运行相关测试（`./run_all_tests.sh --functional-only`）
4. 使用 valgrind 检查内存泄漏（针对 C 组件）
5. 确保在严格模式下没有编译器警告

---

*最后更新：2025年1月*