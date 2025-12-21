#pragma once
#ifndef GUESS_GAME_V2_HPP
#define GUESS_GAME_V2_HPP

#include <string>

namespace GuessGame {
    // 游戏配置常量
    constexpr int MAX_ATTEMPTS = 10;
    constexpr int MIN_RANGE = 1;
    constexpr int MAX_RANGE = 10;
    constexpr int MAX_INPUT_LENGTH = 10;
    
    // 游戏结果枚举
    enum class GameResult {
        WIN,
        LOSE,
        IN_PROGRESS
    };
    
    // 游戏状态结构
    struct GameState {
        int secret_number;
        int attempts;
        int last_guess;
        bool game_over;
        GameResult result;
    };
    
    // 猜数字游戏类
    class GuessingGameV2 {
    private:
        GameState state;
        
        // 私有方法
        void generate_secret_number();
        void update_game_state(int guess);
        bool is_valid_input(const std::string& input, int& value) const;
        
    public:
        // 构造函数
        GuessingGameV2();
        
        // 游戏控制方法
        void start();
        void reset();
        
        // 游戏状态查询
        int get_attempts() const { return state.attempts; }
        int get_last_guess() const { return state.last_guess; }
        int get_secret_number() const { return state.secret_number; }
        bool is_game_over() const { return state.game_over; }
        GameResult get_result() const { return state.result; }
        
        // 用户交互
        int get_safe_input(const std::string& prompt) const;
        void display_welcome() const;
        void display_hint(int guess) const;
        void display_result() const;
    };
    
    // 简化API函数（兼容C风格调用）
    int start_guessing_game_v2();
}

#endif // GUESS_GAME_V2_HPP