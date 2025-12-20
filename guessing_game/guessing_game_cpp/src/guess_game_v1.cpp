#include "guess_game_v1.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <random>

int start_guessing_game_v1() {
    // 显示游戏标题
    std::cout << "=== Guess the Number Game V1 ===" << std::endl;
    std::cout << "Guess the number between 1 and 10!" << std::endl << std::endl;

    // 初始化随机数生成器
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 使用C++11的随机数引擎
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<> dist(1, 10);
    
    // 生成1-10之间的随机数
    int secret_number = 1 + (std::rand() % 10);

    // 游戏主循环
    int user_guess = 0;
    bool valid_input = true;
    while (user_guess != secret_number) {
        if (valid_input) {
            std::cout << "Enter your guess: ";
        }
        
        // C++风格的输入验证
        if (!(std::cin >> user_guess)) {
            std::cout << "Invalid input! Please enter a number." << std::endl;
            std::cin.clear();  // 清除错误状态
            std::cin.ignore(10000, '\n');  // 清除输入缓冲区
            valid_input = false;
            continue;
        }

        valid_input = true;

        // 检查猜测
        if (user_guess < secret_number) {
            std::cout << "Too low! Try again." << std::endl;
        } else if (user_guess > secret_number) {
            std::cout << "Too high! Try again." << std::endl;
        } else {
            std::cout << "Congratulations! You've guessed the number!" << std::endl;
        }
    }

    return 0;
}