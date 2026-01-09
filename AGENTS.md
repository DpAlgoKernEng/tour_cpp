# AGENTS.md - Tour C++ Project Guidelines

This document provides guidelines for AI agents working on the Tour C++ project, including build commands, testing procedures, and code style conventions.

## Project Overview

This is a C/C++ learning project with modular examples and libraries. The project uses CMake as the primary build system and follows modern C++17 standards.

## Build Commands

### CMake Build (Recommended)
```bash
# Configure and build from project root
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)

# Or use the convenience script
./scripts/build.sh
```

### Makefile Build (Legacy)
```bash
# Simple C project build
make

# Clean build artifacts
make clean
```

### Building Specific Components
```bash
# Build only calculator examples
cd programs/calculator_qt
mkdir -p build_cmake && cd build_cmake
cmake .. && make

# Build guessing game
cd programs/guessing_game/guessing_game_cpp
mkdir -p build_cmake && cd build_cmake
cmake .. && make
```

## Lint and Format Commands

### Code Formatting with clang-format
```bash
# Format a single file
clang-format -i path/to/file.cpp

# Format all C/C++ files in project
find . -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" | xargs clang-format -i

# Check formatting without applying
clang-format --dry-run --Werror path/to/file.cpp
```

### Static Analysis with clang-tidy
```bash
# Run clang-tidy on a file
clang-tidy path/to/file.cpp --checks="*" --warnings-as-errors="bugprone-*,clang-analyzer-*"

# Generate compile_commands.json first
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

# Then run clang-tidy using the compilation database
run-clang-tidy -p build/
```

## Testing Commands

### Running All Tests
```bash
# For calculator project
cd programs/calculator_qt/tests
./run_all_tests.sh

# Run specific test types
./run_all_tests.sh --functional-only
./run_all_tests.sh --performance-only
./run_all_tests.sh --security-only
```

### Individual Test Components
```bash
# Functional tests
cd programs/calculator_qt/tests/functional
./functional_test.sh

# Performance tests
cd programs/calculator_qt/tests/performance
./performance_test.sh

# Memory/Valgrind tests (if valgrind installed)
./valgrind_test.sh
```

### Building Test Executables
```bash
# Build and run C version
cd programs/calculator_qt/calculator_c/build_cmake
./scientific_calculator_c

# Build and run C++ version
cd programs/calculator_qt/calculator_cpp/build_cmake
./scientific_calculator_cpp
```

## Code Style Guidelines

### Formatting Rules (from .clang-format)
- **Indentation**: 4 spaces, no tabs
- **Brace style**: Stroustrup (Allman variant)
- **Pointer alignment**: `int* ptr` (left-aligned)
- **Column limit**: 80 characters
- **Include sorting**: Yes, with system headers first
- **Namespace wrapping**: No wrapping after namespace

### Naming Conventions
- **Classes**: `CamelCase` (e.g., `GuessingGameV2`)
- **Structs**: `snake_case` (e.g., `game_state`)
- **Variables**: `camelBack` (e.g., `secretNumber`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_ATTEMPTS`)
- **Functions**: `snake_case` (e.g., `display_welcome()`)
- **Private members**: No prefix, but use `private:` section

### Type Usage
- **Primitive types**: Use standard types (`int32_t`, `size_t` for sizes)
- **Smart pointers**: Prefer `std::unique_ptr` over raw pointers
- **Strings**: Use `std::string` (C++) or `const char*` with length (C)
- **Containers**: Standard library containers (`std::vector`, `std::map`)

### Include Order
1. System headers (`#include <iostream>`)
2. Standard library headers (`#include <vector>`)
3. Third-party library headers
4. Project headers (`#include "colorfmt.h"`)

Example:
```cpp
#include <iostream>
#include <vector>
#include <string>

#include "thirdparty/some_lib.h"

#include "project/header.h"
```

### Error Handling
- **C++**: Use exceptions for exceptional conditions, `std::optional` for optional values
- **C**: Return error codes, use `errno` for system errors
- **Assertions**: Use `assert()` for debug builds, not for runtime error handling
- **Logging**: Use the provided `colorfmt` library for colored output

### Header File Guidelines
```cpp
// Use #pragma once (not include guards)
#pragma once
#ifndef CLASS_NAME_HPP  // Optional backward compatibility
#define CLASS_NAME_HPP

#include <necessary_headers>

namespace ProjectName {

class ClassName {
public:
    ClassName();
    ~ClassName() = default;

    // Public API
    void public_method();

private:
    // Private implementation
    void private_method_();
    int member_variable_;
};

} // namespace ProjectName

#endif // CLASS_NAME_HPP
```

### C++ Specific Guidelines
- **C++ version**: C++17 minimum
- **Auto usage**: Use `auto` when type is obvious from context
- **Const correctness**: Mark methods `const` when they don't modify state
- **References**: Use `const&` for read-only parameters, `&` for output parameters
- **Move semantics**: Implement move constructors/operators when appropriate

### C Specific Guidelines
- **C version**: C11 minimum
- **Struct usage**: Always use `typedef struct` for cleaner syntax
- **Memory management**: Always pair `malloc()` with `free()` in same scope
- **String handling**: Use `strn` functions over `str` functions (`strncpy`, `strncat`)

## Project Structure Notes

- **Source code**: Located in `programs/` and `library/` directories
- **Headers**: `include/` directory contains public headers
- **Third-party**: `thirdparty/`
- **Tests**: `tests/` directory at root, with project-specific tests in `programs/*/tests/`
- **Documentation**: `docs/` directory and `*.md` files throughout

## Important Files for Agents

- `.clang-format` - Formatting configuration
- `.clang-tidy` - Static analysis configuration
- `CMakeLists.txt` - Build configuration
- `Makefile` - Legacy build script
- `programs/calculator_qt/docs/calculator_architecture.md` - Example architecture documentation

## Common Tasks for Agents

1. **Adding new example program**: Create directory in `programs/` with C and C++ versions
2. **Adding utility library**: Create directory in `library/` with CMakeLists.txt
3. **Writing tests**: Follow existing patterns in `programs/calculator_qt/tests/`
4. **Documentation**: Update relevant `.md` files and add architecture diagrams when needed

## Environment Requirements

- **Compiler**: GCC/Clang with C++17 support
- **CMake**: Version 3.22 or higher
- **Tools**: clang-format, clang-tidy, valgrind (optional)
- **System**: Linux/macOS (Windows with WSL or MinGW)

## Quality Checks Before Committing

1. Run `clang-format` on changed files
2. Build all affected components with CMake
3. Run relevant tests (`./run_all_tests.sh --functional-only`)
4. Check for memory leaks with valgrind (for C components)
5. Ensure no compiler warnings in strict mode

---

*Last updated: January 2025*