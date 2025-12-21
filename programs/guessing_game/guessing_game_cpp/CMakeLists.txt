# 包含目录
include_directories(include)

# 自动递归获取src目录下的所有.cpp文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 创建可执行文件
add_executable(guessing_game_cpp ${SOURCES})

# 链接数学库
target_link_libraries(guessing_game_cpp m)