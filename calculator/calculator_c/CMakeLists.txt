cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(scientific_calculator VERSION 1.0)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

# 包含目录
include_directories(include)

# 自动递归获取src目录下的所有.c文件
file(GLOB_RECURSE SOURCES "src/*.c")

# 创建可执行文件
add_executable(scientific_calculator ${SOURCES})

# 链接数学库
target_link_libraries(scientific_calculator m)

# 设置输出目录为build
set_target_properties(scientific_calculator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 安装规则
install(TARGETS scientific_calculator
    RUNTIME DESTINATION bin
)