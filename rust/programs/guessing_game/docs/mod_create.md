# rust 中 mod 创建和使用指南

在Rust中，模块系统用于组织代码结构。以下是多种创建和使用模块的方法：

## 1. **单个文件中的模块定义**

### 内联模块

```rust
// 在 main.rs 或任何 .rs 文件中
mod my_module {
    // 公开的函数
    pub fn public_function() {
        println!("This is public");
    }
  
    // 私有函数（默认）
    fn private_function() {
        println!("This is private");
    }
  
    // 嵌套模块
    pub mod nested {
        pub fn function() {
            println!("Nested module function");
        }
    }
}

fn main() {
    my_module::public_function();
    my_module::nested::function();
    // my_module::private_function(); // 编译错误：私有函数
}
```

## 2. **模块文件系统结构**

### 目录结构示例

```bash
my_project/
├── Cargo.toml
└── src/
    ├── main.rs
    ├── lib.rs        # 库crate的根
    ├── module_a.rs   # 模块文件
    ├── module_b.rs
    └── modules/
        ├── mod.rs    # 子模块声明文件
        ├── sub_a.rs
        └── sub_b.rs
```

## 3. **通过文件创建模块**

### 方法1：直接在根目录声明

**src/main.rs:**

```rust
// 声明模块（会查找 src/module_a.rs）
mod module_a;
// 声明模块（会查找 src/module_b.rs）
mod module_b;

// 使用模块中的内容
use module_a::some_function;
use module_b::SomeStruct;

fn main() {
    some_function();
    let s = SomeStruct::new();
}
```

**src/module_a.rs:**

```rust
pub fn some_function() {
    println!("Function from module_a");
}

pub struct MyStruct {
    pub field: i32,
    private_field: i32,  // 默认私有
}

impl MyStruct {
    pub fn new() -> Self {
        Self {
            field: 0,
            private_field: 0,
        }
    }
  
    // 私有方法
    fn private_method(&self) {
        // ...
    }
}
```

**src/module_b.rs:**

```rust
pub struct SomeStruct {
    pub value: String,
}

impl SomeStruct {
    pub fn new() -> Self {
        Self {
            value: String::from("Hello"),
        }
    }
}
```

### 方法2：模块目录（包含 mod.rs）

**目录结构：**

```bash
src/
├── main.rs
└── network/
    ├── mod.rs      // 必须！声明子模块
    ├── server.rs
    └── client.rs
```

**src/network/mod.rs:**

```rust
// 声明子模块
pub mod server;
pub mod client;

// 重新导出（可选）
pub use server::Server;
pub use client::Client;

// 可以直接在 mod.rs 中写代码
pub fn connect() {
    println!("Network connecting...");
}
```

**src/network/server.rs:**

```rust
pub struct Server {
    pub address: String,
}

impl Server {
    pub fn new(addr: &str) -> Self {
        Self {
            address: addr.to_string(),
        }
    }
}
```

**src/network/client.rs:**

```rust
pub struct Client {
    pub id: u32,
}

impl Client {
    pub fn new(id: u32) -> Self {
        Self { id }
    }
}
```

**src/main.rs:**

```rust
mod network;

// 使用方式1：直接通过路径
use network::server::Server;
use network::client::Client;

// 使用方式2：如果 mod.rs 中有重新导出
// use network::Server;
// use network::Client;

// 使用方式3：通配符（不推荐，除非测试）
// use network::*;

fn main() {
    let server = Server::new("127.0.0.1:8080");
    let client = Client::new(1);
  
    // 使用 mod.rs 中的函数
    network::connect();
  
    println!("Server address: {}", server.address);
    println!("Client ID: {}", client.id);
}
```

## 4. **库（Library Crate）中的模块**

**src/lib.rs:**

```rust
// 库的公共API
pub mod math;
pub mod string_utils;

// 私有模块（只能在库内部使用）
mod internal_utils;

// 重新导出，简化API
pub use math::add;
pub use math::multiply;
```

**src/math.rs:**

```rust
pub fn add(a: i32, b: i32) -> i32 {
    a + b
}

pub fn multiply(a: i32, b: i32) -> i32 {
    a * b
}

// 私有函数
fn internal_calc() {
    // ...
}
```

## 5. **高级用法**

### 模块可见性控制

```rust
mod outer {
    pub mod inner {
        pub(in crate::outer) fn semi_private() {
            // 只能在 outer 模块内访问
        }
      
        pub(super) fn parent_visible() {
            // 只能在父模块（outer）中访问
        }
      
        pub(crate) fn crate_visible() {
            // 在整个crate中可见
        }
      
        pub fn fully_public() {
            // 完全公开
        }
    }
  
    pub fn use_inner() {
        inner::semi_private();      // OK
        inner::parent_visible();    // OK
        inner::crate_visible();     // OK
        inner::fully_public();      // OK
    }
}

fn main() {
    outer::inner::fully_public();   // OK
    outer::inner::crate_visible();  // OK
    // outer::inner::semi_private();  // 编译错误
    // outer::inner::parent_visible(); // 编译错误
}
```

### 使用 `pub use` 重新导出

```rust
// src/utils/mod.rs
mod string_utils;
mod math_utils;

// 重新导出，创建更简洁的公共API
pub use string_utils::{to_uppercase, to_lowercase};
pub use math_utils::{add, subtract};

// 外部使用时：
// use utils::add;  // 而不是 utils::math_utils::add
```

## 6. **完整示例项目**

**目录结构：**

```bash
my_app/
├── Cargo.toml
└── src/
    ├── main.rs
    ├── config.rs
    ├── database/
    │   ├── mod.rs
    │   ├── connection.rs
    │   └── query.rs
    └── api/
        ├── mod.rs
        ├── routes.rs
        └── handlers/
            ├── mod.rs
            ├── user.rs
            └── product.rs
```

**src/main.rs:**

```rust
mod config;
mod database;
mod api;

use config::Config;
use database::connect;
use api::start_server;

fn main() {
    let config = Config::load();
  
    // 建立数据库连接
    let db = connect(&config.database_url);
  
    // 启动API服务器
    start_server(config.port, db);
}
```

**src/database/mod.rs:**

```rust
pub mod connection;
pub mod query;

pub use connection::connect;
pub use query::{Query, QueryBuilder};

// 模块内部函数
fn internal_validate() {
    // ...
}
```

**src/database/connection.rs:**

```rust
pub struct Database {
    pub url: String,
}

pub fn connect(url: &str) -> Database {
    println!("Connecting to database: {}", url);
    Database {
        url: url.to_string(),
    }
}
```

## 7. **最佳实践**

1. **每个文件一个模块**：保持模块文件专注单一职责
2. **合理使用 `mod.rs`**：只在目录模块中使用
3. **明确的可见性**：默认私有，按需公开
4. **使用重新导出**：简化公共API
5. **避免深层嵌套**：通常2-3层深度最佳
6. **一致的命名**：模块名使用蛇形命名法（snake_case）

## 8. **常见错误和解决**

```rust
// 错误：找不到模块
// mod my_module;  // 如果 my_module.rs 不存在会编译错误

// 正确：先创建文件，再声明模块

// 错误：循环依赖
// mod a;  // 在 a.rs 中 mod b;
// mod b;  // 在 b.rs 中 mod a;

// 解决：将公共代码提取到第三个模块
```

通过合理使用模块系统，可以使Rust代码结构清晰、易于维护，并且充分利用Rust的封装特性。
