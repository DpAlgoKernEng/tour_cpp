# rust中的宏定义实现函数版本的选择

```rust

mod core;

use core::guess_v1::guess_game_v1;
use core::guess_v2::guess_game_v2;

fn main() {
    guess_game_v2();
    guess_game_v1();
}
```

## 需求

希望通过宏来实现根据版本号选择执行不同版本的函数，比如：定义v1的时候执行v1，v2执行v2版本函数，在Rust中，宏可以实现这样的需求。以下是几种方法：

## 1. **使用特征和宏（最灵活）**

**src/core/guess_v1.rs:**

```rust
pub fn guess_game_v1() {
    println!("=== 猜数字游戏 v1 ===");
    // v1 的实现
}
```

**src/core/guess_v2.rs:**

```rust
pub fn guess_game_v2() {
    println!("=== 猜数字游戏 v2 ===");
    // v2 的实现
}
```

**src/core/mod.rs:**

```rust
pub mod guess_v1;
pub mod guess_v2;

// 定义一个特征
pub trait GameVersion {
    fn run(&self);
}

// 为每个版本实现特征
pub struct Version1;
impl GameVersion for Version1 {
    fn run(&self) {
        guess_v1::guess_game_v1();
    }
}

pub struct Version2;
impl GameVersion for Version2 {
    fn run(&self) {
        guess_v2::guess_game_v2();
    }
}
```

**src/main.rs:**

```rust
mod core;
use core::{Version1, Version2, GameVersion};

fn main() {
    // 使用特征对象
    let version = if std::env::args().any(|arg| arg == "--v2") {
        Box::new(Version2) as Box<dyn GameVersion>
    } else {
        Box::new(Version1) as Box<dyn GameVersion>
    };
  
    version.run();
}
```

## 2. **使用宏实现版本选择**

**src/core/mod.rs:**

```rust
pub mod guess_v1;
pub mod guess_v2;

// 定义一个宏来选择版本
#[macro_export]
macro_rules! run_game {
    (v1) => {
        $crate::core::guess_v1::guess_game_v1()
    };
    (v2) => {
        $crate::core::guess_v2::guess_game_v2()
    };
    ($version:expr) => {
        match $version {
            1 => $crate::core::guess_v1::guess_game_v1(),
            2 => $crate::core::guess_v2::guess_game_v2(),
            _ => panic!("Unknown version"),
        }
    };
}
```

**src/main.rs:**

```rust
mod core;

// 使用宏
fn main() {
    // 方法1：直接指定版本
    run_game!(v1);  // 运行v1
    // run_game!(v2);  // 运行v2
  
    // 方法2：根据条件选择
    let version = if cfg!(feature = "v2") { 2 } else { 1 };
    run_game!(version);
}
```

## 3. **使用函数指针和宏**

**src/core/mod.rs:**

```rust
pub mod guess_v1;
pub mod guess_v2;

// 导出函数指针类型
pub type GameFunc = fn();

// 获取指定版本的函数
pub fn get_game_function(version: u8) -> GameFunc {
    match version {
        1 => guess_v1::guess_game_v1,
        2 => guess_v2::guess_game_v2,
        _ => panic!("Invalid version"),
    }
}

// 宏：执行指定版本
#[macro_export]
macro_rules! exec_game {
    ($v:literal) => {
        (match $v {
            1 => $crate::core::guess_v1::guess_game_v1,
            2 => $crate::core::guess_v2::guess_game_v2,
            _ => panic!("Invalid version {}", $v),
        })()
    };
}
```

**src/main.rs:**

```rust
mod core;
use core::get_game_function;

fn main() {
    // 方法1：使用函数指针
    let game_func = get_game_function(1);  // 改为2执行v2
    game_func();
  
    // 方法2：使用宏
    exec_game!(1);  // 执行v1
    // exec_game!(2);  // 执行v2
}
```

## 4. **使用编译时条件（Cargo features）**

**Cargo.toml:**

```toml
[package]
name = "guessing_game"
version = "0.1.0"

[features]
default = ["v1"]
v1 = []
v2 = []
```

**src/core/mod.rs:**

```rust
pub mod guess_v1;
pub mod guess_v2;

// 根据特性编译不同版本
#[cfg(feature = "v1")]
pub use guess_v1::guess_game_v1 as guess_game;

#[cfg(feature = "v2")]
pub use guess_v2::guess_game_v2 as guess_game;
```

**src/main.rs:**

```rust
mod core;

fn main() {
    // 根据编译特性自动选择版本
    core::guess_game();
}
```

**运行命令：**

```bash
# 默认运行v1
cargo run

# 运行v2
cargo run --features v2

# 运行v1
cargo run --features v1
```

## 5. **最符合你要求的宏定义**

**src/core/mod.rs:**

```rust
pub mod guess_v1;
pub mod guess_v2;

// 宏定义：定义v1的时候执行v1，v2执行v2
#[macro_export]
macro_rules! define_and_run {
    (v1) => {
        mod guess_v1;
        guess_v1::guess_game_v1()
    };
    (v2) => {
        mod guess_v2;
        guess_v2::guess_game_v2()
    };
    ($version:ident) => {
        mod $version;
        $version::guess_game_$version()
    };
}
```

**src/main.rs:**

```rust
mod core;

// 使用宏定义并执行
fn main() {
    // 这会定义 guess_v1 模块并执行 guess_game_v1()
    core::define_and_run!(v1);
  
    // 或者执行v2
    // core::define_and_run!(v2);
}
```

## 6. **简化版（直接在main.rs中使用宏）**

**src/core/guess_v1.rs:**

```rust
pub fn execute() {
    println!("=== 执行猜数字游戏 v1 ===");
    // v1的具体实现
}
```

**src/core/guess_v2.rs:**

```rust
pub fn execute() {
    println!("=== 执行猜数字游戏 v2 ===");
    // v2的具体实现
}
```

**src/main.rs:**

```rust
// 定义一个宏来选择执行哪个版本
macro_rules! run_version {
    (v1) => {{
        mod core {
            pub mod guess_v1;
        }
        use core::guess_v1;
        guess_v1::execute()
    }};
    (v2) => {{
        mod core {
            pub mod guess_v2;
        }
        use core::guess_v2;
        guess_v2::execute()
    }};
}

fn main() {
    // 根据条件选择版本
    let version = "v1"; // 改为 "v2" 执行v2
  
    match version {
        "v1" => run_version!(v1),
        "v2" => run_version!(v2),
        _ => panic!("未知版本"),
    }
}
```

## 7. **使用过程宏（高级）**

如果你想要更强大的功能，可以使用过程宏：

**Cargo.toml:**

```toml
[package]
name = "guessing_game"
version = "0.1.0"

[lib]
proc-macro = true

[dependencies]
syn = "2.0"
quote = "1.0"
```

**src/lib.rs:**

```rust
use proc_macro::TokenStream;
use quote::quote;
use syn::{parse_macro_input, Ident};

#[proc_macro]
pub fn run_game_version(input: TokenStream) -> TokenStream {
    let version = parse_macro_input!(input as Ident);
    let version_str = version.to_string();
  
    let expanded = match version_str.as_str() {
        "v1" => quote! {
            mod core {
                pub mod guess_v1;
            }
            use core::guess_v1;
            guess_v1::guess_game_v1()
        },
        "v2" => quote! {
            mod core {
                pub mod guess_v2;
            }
            use core::guess_v2;
            guess_v2::guess_game_v2()
        },
        _ => panic!("Unknown game version: {}", version_str),
    };
  
    expanded.into()
}
```

**src/main.rs:**

```rust
use guessing_game::run_game_version;

fn main() {
    // 使用过程宏
    run_game_version!(v1);  // 执行v1
    // run_game_version!(v2);  // 执行v2
}
```

## **推荐方案**

对于你的需求，最简单实用的方案是第2或第3种方法。建议：

```rust
mod core;

// 定义一个简单的选择宏
macro_rules! select_game {
    (1) => {
        {
            use core::guess_v1::guess_game_v1;
            guess_game_v1()
        }
    };
    (2) => {
        {
            use core::guess_v2::guess_game_v2;
            guess_game_v2()
        }
    };
}

fn main() {
    // 执行v1
    select_game!(1);
  
    // 或者执行v2
    // select_game!(2);
}
```

这样既清晰又灵活，可以根据需要轻松切换版本。
