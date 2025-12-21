#include "guess_game_v2.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <limits>
#include <random>

using namespace GuessGame;

// 构造函数
GuessingGameV2::GuessingGameV2() {
    reset();
}

// 重置游戏状态
void GuessingGameV2::reset() {
    state.attempts = 0;
    state.game_over = false;
    state.result = GameResult::IN_PROGRESS;
    generate_secret_number();
}

// 生成随机秘密数字（使用C++11随机数引擎）
void GuessingGameV2::generate_secret_number() {
    // 使用C++11的随机数引擎，更均匀的分布
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(MIN_RANGE, MAX_RANGE);
    state.secret_number = dist(gen);
}

// 验证输入字符串是否有效
bool GuessingGameV2::is_valid_input(const std::string& input, int& value) const {
    // 检查输入是否过长
    if (input.length() > MAX_INPUT_LENGTH) {
        std::cout << "Input too long! Maximum " << MAX_INPUT_LENGTH 
                  << " characters allowed.\n";
        return false;
    }
    
    // 检查是否为空
    if (input.empty()) {
        std::cout << "Empty input. Please enter a number.\n";
        return false;
    }
    
    // 尝试解析为整数
    std::istringstream iss(input);
    long temp;
    
    // 使用流解析，检查是否完全是数字
    if (!(iss >> temp)) {
        std::cout << "Invalid input. Please enter a valid number.\n";
        return false;
    }
    
    // 检查是否有剩余字符（非数字字符）
    std::string remaining;
    if (iss >> remaining) {
        std::cout << "Invalid characters in input: " << remaining << "\n";
        return false;
    }
    
    // 检查范围
    if (temp < MIN_RANGE || temp > MAX_RANGE) {
        std::cout << "Please enter a number between " << MIN_RANGE 
                  << " and " << MAX_RANGE << ".\n";
        return false;
    }
    
    value = static_cast<int>(temp);
    return true;
}

// 安全的整数输入函数
int GuessingGameV2::get_safe_input(const std::string& prompt) const {
    std::string input;
    int value;
    
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        
        if (std::cin.eof()) {
            std::cout << "\nInput stream closed. Exiting.\n";
            std::exit(0);
        }
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Input error. Please try again.\n";
            continue;
        }
        
        if (is_valid_input(input, value)) {
            return value;
        }
    }
}

// 显示欢迎信息
void GuessingGameV2::display_welcome() const {
    std::cout << "\n=== Guess the Number Game V2 (C++) ===\n";
    std::cout << "Guess the number between " << MIN_RANGE 
              << " and " << MAX_RANGE << "!\n";
    std::cout << "You have " << MAX_ATTEMPTS << " attempts.\n\n";
}

// 显示猜测提示
void GuessingGameV2::display_hint(int guess) const {
    if (guess < state.secret_number) {
        std::cout << "Too small! ";
    } else if (guess > state.secret_number) {
        std::cout << "Too big! ";
    }
    
    std::cout << "Attempts used: " << state.attempts 
              << "/" << MAX_ATTEMPTS << "\n";
}

// 显示游戏结果
void GuessingGameV2::display_result() const {
    std::cout << "\n";
    switch (state.result) {
        case GameResult::WIN:
            std::cout << "🎉 Congratulations! You win! 🎉\n";
            std::cout << "The secret number was " << state.secret_number << "\n";
            std::cout << "It took you " << state.attempts << " attempts.\n";
            break;
            
        case GameResult::LOSE:
            std::cout << "💀 Game Over! 💀\n";
            std::cout << "You've used all " << MAX_ATTEMPTS << " attempts.\n";
            std::cout << "The secret number was " << state.secret_number << "\n";
            break;
            
        case GameResult::IN_PROGRESS:
            // 不应到达这里
            break;
    }
    std::cout << "\n";
}

// 更新游戏状态
void GuessingGameV2::update_game_state(int guess) {
    state.last_guess = guess;
    state.attempts++;
    
    if (guess == state.secret_number) {
        state.game_over = true;
        state.result = GameResult::WIN;
    } else if (state.attempts >= MAX_ATTEMPTS) {
        state.game_over = true;
        state.result = GameResult::LOSE;
    }
}

// 开始游戏
void GuessingGameV2::start() {
    reset();
    display_welcome();
    
    while (!state.game_over) {
        // 获取用户输入
        int guess = get_safe_input("Your guess (" + 
                                  std::to_string(MIN_RANGE) + 
                                  "-" + std::to_string(MAX_RANGE) + 
                                  "): ");
        
        // 显示当前猜测
        std::cout << "You guessed: " << guess << "\n";
        
        // 更新游戏状态
        update_game_state(guess);
        
        // 显示提示或结果
        if (!state.game_over) {
            display_hint(guess);
        } else {
            display_result();
        }
        
        std::cout << "\n";
    }
}

// 兼容C风格的API函数
int GuessGame::start_guessing_game_v2() {
    GuessingGameV2 game;
    game.start();
    return 0;
}