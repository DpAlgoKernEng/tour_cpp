# 猜数字游戏 第三版实现说明

## **需求**

按照代码优化建议实现第三版猜数字游戏，将游戏逻辑模块化、可配置化：

## **目录结构**

```bash
src/
├── main.rs
├── lib.rs
└── game/
    ├── mod.rs
    ├── config.rs
    ├── state.rs
    ├── validator.rs
    ├── input.rs
    ├── v1.rs
    ├── v2.rs
    └── v3.rs
```

## **1. 游戏配置模块**

**src/game/config.rs:**

```rust
use serde::{Deserialize, Serialize};

/// 游戏配置结构体
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameConfig {
    pub max_attempts: u32,
    pub max_input_length: usize,
    pub range: (u32, u32),
    pub allow_hints: bool,
    pub difficulty: Difficulty,
}

/// 游戏难度级别
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
pub enum Difficulty {
    Easy,      // 1-10, 无限尝试
    Normal,    // 1-50, 10次尝试
    Hard,      // 1-100, 5次尝试
}

impl Default for GameConfig {
    fn default() -> Self {
        Self {
            max_attempts: 10,
            max_input_length: 10,
            range: (1, 10),
            allow_hints: true,
            difficulty: Difficulty::Normal,
        }
    }
}

impl GameConfig {
    /// 根据难度创建配置
    pub fn from_difficulty(difficulty: Difficulty) -> Self {
        match difficulty {
            Difficulty::Easy => Self {
                max_attempts: u32::MAX,
                range: (1, 10),
                ..Default::default()
            },
            Difficulty::Normal => Self {
                max_attempts: 10,
                range: (1, 50),
                ..Default::default()
            },
            Difficulty::Hard => Self {
                max_attempts: 5,
                range: (1, 100),
                ..Default::default()
            },
        }
    }
  
    /// 获取随机数范围
    pub fn get_random_range(&self) -> std::ops::RangeInclusive<u32> {
        self.range.0..=self.range.1
    }
  
    /// 显示配置信息
    pub fn display_info(&self) -> String {
        format!(
            "难度: {:?}, 范围: {}-{}, 最大尝试: {}, 提示: {}",
            self.difficulty,
            self.range.0,
            self.range.1,
            if self.max_attempts == u32::MAX {
                "无限".to_string()
            } else {
                self.max_attempts.to_string()
            },
            if self.allow_hints { "开启" } else { "关闭" }
        )
    }
}
```

## **2. 输入验证模块**

**src/game/validator.rs:**

```rust
use crate::game::config::GameConfig;

/// 输入验证错误类型
#[derive(Debug, Clone, PartialEq)]
pub enum InputError {
    TooLong(usize),
    Empty,
    InvalidNumber(String),
    OutOfRange(u32, u32),
}

impl std::fmt::Display for InputError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::TooLong(max) => write!(f, "输入过长！最大允许 {} 个字符", max),
            Self::Empty => write!(f, "输入不能为空"),
            Self::InvalidNumber(msg) => write!(f, "无效数字: {}", msg),
            Self::OutOfRange(min, max) => write!(f, "数字超出范围！必须在 {}-{} 之间", min, max),
        }
    }
}

/// 输入验证器
pub struct InputValidator {
    config: GameConfig,
}

impl InputValidator {
    pub fn new(config: GameConfig) -> Self {
        Self { config }
    }
  
    /// 验证用户输入
    pub fn validate(&self, input: &str) -> Result<u32, InputError> {
        // 检查是否为空
        let trimmed = input.trim();
        if trimmed.is_empty() {
            return Err(InputError::Empty);
        }
      
        // 检查长度
        if trimmed.len() > self.config.max_input_length {
            return Err(InputError::TooLong(self.config.max_input_length));
        }
      
        // 解析数字
        let number: u32 = match trimmed.parse() {
            Ok(num) => num,
            Err(e) => return Err(InputError::InvalidNumber(e.to_string())),
        };
      
        // 检查范围
        let (min, max) = self.config.range;
        if number < min || number > max {
            return Err(InputError::OutOfRange(min, max));
        }
      
        Ok(number)
    }
  
    /// 提供提示（如果开启）
    pub fn provide_hint(&self, guess: u32, secret: u32, attempts: u32) -> Option<String> {
        if !self.config.allow_hints {
            return None;
        }
      
        let diff = (guess as i32 - secret as i32).abs() as u32;
      
        match (diff, attempts) {
            (0, _) => Some("正确！".to_string()),
            (1..=2, _) => Some("非常接近！".to_string()),
            (3..=5, _) => Some("有点接近".to_string()),
            (6..=10, _) => Some("离得有点远".to_string()),
            (_, 1) => Some("第一次尝试，加油！".to_string()),
            (_, attempt) if attempt >= self.config.max_attempts / 2 => {
                Some(format!("还剩 {} 次尝试", self.config.max_attempts - attempt))
            }
            _ => None,
        }
    }
}
```

## **3. 输入处理模块**

**src/game/input.rs:**

```rust
use std::io::{self, Write};
use crate::game::validator::{InputValidator, InputError};
use crate::game::config::GameConfig;

/// 输入处理器
pub struct InputHandler {
    validator: InputValidator,
}

impl InputHandler {
    pub fn new(config: GameConfig) -> Self {
        Self {
            validator: InputValidator::new(config),
        }
    }
  
    /// 读取用户输入
    pub fn read_input(&self, prompt: &str) -> Result<u32, InputError> {
        print!("{}", prompt);
        io::stdout().flush().unwrap();
      
        let mut input = String::new();
        if io::stdin().read_line(&mut input).is_err() {
            return Err(InputError::Empty);
        }
      
        self.validator.validate(&input)
    }
  
    /// 带重试的输入读取
    pub fn read_input_with_retry(&self, prompt: &str, max_retries: u32) -> Option<u32> {
        let mut retries = 0;
      
        loop {
            if retries >= max_retries {
                println!("重试次数过多，放弃输入。");
                return None;
            }
          
            match self.read_input(prompt) {
                Ok(num) => return Some(num),
                Err(err) => {
                    println!("错误: {}", err);
                    println!("请重新输入 (重试 {}/{})", retries + 1, max_retries);
                    retries += 1;
                }
            }
        }
    }
  
    /// 读取是/否选择
    pub fn read_yes_no(&self, prompt: &str) -> bool {
        print!("{} (y/n): ", prompt);
        io::stdout().flush().unwrap();
      
        let mut input = String::new();
        if io::stdin().read_line(&mut input).is_err() {
            return false;
        }
      
        input.trim().to_lowercase() == "y" || input.trim().to_lowercase() == "yes"
    }
}
```

## **4. 游戏状态模块**

**src/game/state.rs:**

```rust
use rand::Rng;
use crate::game::config::{GameConfig, Difficulty};

/// 游戏状态
#[derive(Debug)]
pub struct GameState {
    pub secret_number: u32,
    pub attempts: u32,
    pub guesses: Vec<u32>,
    pub config: GameConfig,
    pub is_game_over: bool,
    pub is_won: bool,
}

impl GameState {
    /// 创建新游戏
    pub fn new(config: GameConfig) -> Self {
        let secret_number = rand::thread_rng().gen_range(config.get_random_range());
      
        Self {
            secret_number,
            attempts: 0,
            guesses: Vec::new(),
            config,
            is_game_over: false,
            is_won: false,
        }
    }
  
    /// 从难度创建游戏
    pub fn from_difficulty(difficulty: Difficulty) -> Self {
        let config = GameConfig::from_difficulty(difficulty);
        Self::new(config)
    }
  
    /// 处理猜测
    pub fn make_guess(&mut self, guess: u32) -> GameResult {
        self.attempts += 1;
        self.guesses.push(guess);
      
        if guess == self.secret_number {
            self.is_won = true;
            self.is_game_over = true;
            return GameResult::Win(self.attempts);
        }
      
        if self.attempts >= self.config.max_attempts {
            self.is_game_over = true;
            return GameResult::Lose(self.secret_number);
        }
      
        GameResult::Continue
    }
  
    /// 获取游戏统计信息
    pub fn get_stats(&self) -> GameStats {
        GameStats {
            attempts: self.attempts,
            max_attempts: self.config.max_attempts,
            guesses: self.guesses.clone(),
            config: self.config.clone(),
            secret_number: if self.is_game_over {
                Some(self.secret_number)
            } else {
                None
            },
        }
    }
  
    /// 检查是否还能继续
    pub fn can_continue(&self) -> bool {
        !self.is_game_over
    }
}

/// 游戏结果
#[derive(Debug, Clone)]
pub enum GameResult {
    Win(u32),
    Lose(u32),
    Continue,
}

/// 游戏统计
#[derive(Debug, Clone)]
pub struct GameStats {
    pub attempts: u32,
    pub max_attempts: u32,
    pub guesses: Vec<u32>,
    pub config: GameConfig,
    pub secret_number: Option<u32>,
}

impl GameStats {
    /// 显示统计信息
    pub fn display(&self) -> String {
        let mut result = format!(
            "游戏配置: {}\n尝试次数: {}/{}",
            self.config.display_info(),
            self.attempts,
            if self.max_attempts == u32::MAX {
                "无限".to_string()
            } else {
                self.max_attempts.to_string()
            }
        );
      
        if !self.guesses.is_empty() {
            result.push_str(&format!("\n猜测历史: {:?}", self.guesses));
        }
      
        if let Some(secret) = self.secret_number {
            result.push_str(&format!("\n秘密数字: {}", secret));
        }
      
        result
    }
}
```

## **5. 游戏核心模块（v3）**

**src/game/v3.rs:**

```rust
use crate::game::{
    GameState, GameResult, GameStats,
    config::{GameConfig, Difficulty},
    input::InputHandler,
    validator::InputValidator,
};
use std::io::{self, Write};

/// 猜数字游戏第三版（模块化、可配置版）
pub struct GuessGameV3 {
    state: GameState,
    input_handler: InputHandler,
    validator: InputValidator,
}

impl GuessGameV3 {
    /// 创建新游戏
    pub fn new(config: GameConfig) -> Self {
        let state = GameState::new(config.clone());
        let input_handler = InputHandler::new(config.clone());
        let validator = InputValidator::new(config);
      
        Self {
            state,
            input_handler,
            validator,
        }
    }
  
    /// 从难度创建游戏
    pub fn from_difficulty(difficulty: Difficulty) -> Self {
        let config = GameConfig::from_difficulty(difficulty);
        Self::new(config)
    }
  
    /// 运行游戏主循环
    pub fn run(&mut self) -> GameStats {
        println!("🎮 猜数字游戏 v3 🎮");
        println!("{}", self.state.config.display_info());
        println!("=".repeat(40));
      
        while self.state.can_continue() {
            // 显示当前状态
            self.display_current_status();
          
            // 读取输入
            let prompt = format!("第 {} 次尝试，请输入你的猜测: ", self.state.attempts + 1);
            let guess = match self.input_handler.read_input_with_retry(&prompt, 3) {
                Some(num) => num,
                None => {
                    println!("输入失败次数过多，游戏结束。");
                    self.state.is_game_over = true;
                    break;
                }
            };
          
            // 处理猜测
            let result = self.state.make_guess(guess);
          
            // 显示结果
            self.display_guess_result(guess, &result);
          
            // 提供提示
            if let Some(hint) = self.validator.provide_hint(
                guess,
                self.state.secret_number,
                self.state.attempts
            ) {
                println!("💡 提示: {}", hint);
            }
          
            println!("-".repeat(40));
        }
      
        self.display_final_result();
        self.state.get_stats()
    }
  
    /// 显示当前状态
    fn display_current_status(&self) {
        let stats = self.state.get_stats();
      
        println!("📊 当前状态:");
        println!("  尝试次数: {}/{}", 
            self.state.attempts,
            if self.state.config.max_attempts == u32::MAX {
                "无限".to_string()
            } else {
                self.state.config.max_attempts.to_string()
            }
        );
      
        if !stats.guesses.is_empty() {
            print!("  猜测历史: ");
            for (i, guess) in stats.guesses.iter().enumerate() {
                let indicator = match guess.cmp(&self.state.secret_number) {
                    std::cmp::Ordering::Less => "↑",
                    std::cmp::Ordering::Greater => "↓",
                    std::cmp::Ordering::Equal => "✓",
                };
                print!("{}{} ", guess, indicator);
            }
            println!();
        }
    }
  
    /// 显示猜测结果
    fn display_guess_result(&self, guess: u32, result: &GameResult) {
        match guess.cmp(&self.state.secret_number) {
            std::cmp::Ordering::Less => println!("📈 {} 太小了！", guess),
            std::cmp::Ordering::Greater => println!("📉 {} 太大了！", guess),
            std::cmp::Ordering::Equal => println!("🎉 {} 正确！", guess),
        }
      
        match result {
            GameResult::Win(attempts) => println!("🎊 恭喜！你在 {} 次尝试中猜对了！", attempts),
            GameResult::Lose(secret) => println!("💔 游戏结束！秘密数字是 {}", secret),
            GameResult::Continue => (),
        }
    }
  
    /// 显示最终结果
    fn display_final_result(&self) {
        println!("=".repeat(40));
        if self.state.is_won {
            println!("🏆 胜利！");
            println!("你在 {} 次尝试中猜对了数字 {}",
                self.state.attempts,
                self.state.secret_number
            );
        } else {
            println!("💀 失败");
            println!("秘密数字是 {}", self.state.secret_number);
        }
        println!("=".repeat(40));
    }
  
    /// 导出游戏数据（可用于保存/加载）
    pub fn export_data(&self) -> GameData {
        GameData {
            state: self.state.get_stats(),
            config: self.state.config.clone(),
        }
    }
}

/// 游戏数据（可用于序列化）
#[derive(Debug, Clone)]
pub struct GameData {
    pub state: GameStats,
    pub config: GameConfig,
}
```

## **6. 模块声明和重导出**

**src/game/mod.rs:**

```rust
pub mod config;
pub mod state;
pub mod validator;
pub mod input;
pub mod v1;
pub mod v2;
pub mod v3;

// 重导出常用类型
pub use config::{GameConfig, Difficulty};
pub use state::{GameState, GameResult, GameStats};
pub use validator::InputError;
pub use input::InputHandler;
pub use v1::guess_game_v1;
pub use v2::guess_game_v2;
pub use v3::{GuessGameV3, GameData};
```

## **7. 主程序入口**

**src/main.rs:**

```rust
mod game;

use game::{GuessGameV3, GameConfig, Difficulty};

fn main() {
    println!("🎮 猜数字游戏 - 版本选择 🎮");
    println!("1. 简单模式 (1-10, 无限尝试)");
    println!("2. 普通模式 (1-50, 10次尝试)");
    println!("3. 困难模式 (1-100, 5次尝试)");
    println!("4. 自定义模式");
  
    let choice = read_choice();
  
    match choice {
        1 => {
            let mut game = GuessGameV3::from_difficulty(Difficulty::Easy);
            let stats = game.run();
            println!("\n游戏统计:\n{}", stats.display());
        }
        2 => {
            let mut game = GuessGameV3::from_difficulty(Difficulty::Normal);
            let stats = game.run();
            println!("\n游戏统计:\n{}", stats.display());
        }
        3 => {
            let mut game = GuessGameV3::from_difficulty(Difficulty::Hard);
            let stats = game.run();
            println!("\n游戏统计:\n{}", stats.display());
        }
        4 => {
            let config = create_custom_config();
            let mut game = GuessGameV3::new(config);
            let stats = game.run();
            println!("\n游戏统计:\n{}", stats.display());
        }
        _ => println!("无效选择"),
    }
}

fn read_choice() -> u32 {
    use std::io::{self, Write};
  
    loop {
        print!("请选择游戏模式 (1-4): ");
        io::stdout().flush().unwrap();
      
        let mut input = String::new();
        if io::stdin().read_line(&mut input).is_err() {
            continue;
        }
      
        match input.trim().parse() {
            Ok(num) if (1..=4).contains(&num) => return num,
            _ => println!("请输入 1-4 之间的数字"),
        }
    }
}

fn create_custom_config() -> GameConfig {
    use std::io::{self, Write};
  
    println!("🎛️ 自定义游戏配置");
  
    let max_attempts = loop {
        print!("请输入最大尝试次数 (输入0表示无限): ");
        io::stdout().flush().unwrap();
      
        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();
      
        match input.trim().parse::<u32>() {
            Ok(0) => break u32::MAX,
            Ok(n) if n > 0 => break n,
            _ => println!("请输入有效的正数"),
        }
    };
  
    let range = loop {
        print!("请输入数字范围 (格式: 最小值,最大值): ");
        io::stdout().flush().unwrap();
      
        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();
      
        let parts: Vec<&str> = input.trim().split(',').collect();
        if parts.len() == 2 {
            match (parts[0].parse::<u32>(), parts[1].parse::<u32>()) {
                (Ok(min), Ok(max)) if min < max => break (min, max),
                _ => println!("请输入有效的范围，如: 1,100"),
            }
        } else {
            println!("请输入两个用逗号分隔的数字");
        }
    };
  
    let allow_hints = loop {
        print!("是否开启提示功能？ (y/n): ");
        io::stdout().flush().unwrap();
      
        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();
      
        match input.trim().to_lowercase().as_str() {
            "y" | "yes" => break true,
            "n" | "no" => break false,
            _ => println!("请输入 y/n"),
        }
    };
  
    GameConfig {
        max_attempts,
        max_input_length: 20,
        range,
        allow_hints,
        difficulty: Difficulty::Normal,
    }
}
```

## **8. 库文件（可选）**

**src/lib.rs:**

```rust
pub mod game;

pub use game::{
    GuessGameV3, GameConfig, Difficulty, GameState, GameResult,
    GameStats, InputError, InputHandler, GameData,
};

/// 游戏工厂：创建不同版本的游戏
pub struct GameFactory;

impl GameFactory {
    pub fn create_v1() -> impl Fn() {
        || game::guess_game_v1()
    }
  
    pub fn create_v2() -> impl Fn() {
        || game::guess_game_v2()
    }
  
    pub fn create_v3(config: GameConfig) -> GuessGameV3 {
        GuessGameV3::new(config)
    }
}
```

## **Cargo.toml 依赖**

```toml
[dependencies]
rand = "0.8"
serde = { version = "1.0", features = ["derive"] }
```

## **第三版的优势**

1. **完全模块化**：各组件职责清晰，易于测试和维护
2. **高度可配置**：支持难度选择、自定义配置
3. **错误处理完善**：专门的错误类型和用户友好提示
4. **状态管理**：清晰的状态机和统计数据
5. **可扩展性**：易于添加新功能（如保存/加载、排行榜等）
6. **用户体验**：丰富的提示和反馈
7. **代码复用**：提取了公共逻辑，避免重复代码

## **使用方法**

```bash
# 运行游戏
cargo run

# 测试特定模块
cargo test

# 构建库
cargo build --lib
```

这个第三版实现了完整的模块化架构，每个组件都可以独立测试和重用，是生产级应用的良好起点。
