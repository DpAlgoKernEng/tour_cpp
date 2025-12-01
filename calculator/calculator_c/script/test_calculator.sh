#!/bin/bash

# 测试C语言科学计算器

echo "Testing scientific calculator..."

# 测试基本算术运算
echo "2 + 3 * 4" | ../build_cmake/scientific_calculator > output.txt 2>&1 &
sleep 1
killall scientific_calculator 2>/dev/null

# 检查输出
if grep -q "14" output.txt; then
    echo "Basic arithmetic test passed"
else
    echo "Basic arithmetic test failed"
fi

# 测试科学函数
echo "sin(pi/2)" | ../build_cmake/scientific_calculator > output.txt 2>&1 &
sleep 1
killall scientific_calculator 2>/dev/null

# 检查输出
if grep -q "1" output.txt; then
    echo "Scientific function test passed"
else
    echo "Scientific function test failed"
fi

# 测试常量
echo "2 * pi" | ../build_cmake/scientific_calculator > output.txt 2>&1 &
sleep 1
killall scientific_calculator 2>/dev/null

# 检查输出
if grep -q "6.28" output.txt; then
    echo "Constants test passed"
else
    echo "Constants test failed"
fi

# 清理
rm -f output.txt

echo "Tests completed."