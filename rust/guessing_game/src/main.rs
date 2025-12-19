use std::io;
use std::io::Write;
use std::cmp::Ordering;
use rand::Rng;

fn guess_game_opt() {
    println!("Guess the number!");
    
    let secret_number = rand::thread_rng().gen_range(1..=100);
    const MAX_ATTEMPTS: u32 = 10;
    const MAX_INPUT_LENGTH: usize = 10;
    let mut attempts = 0;

    loop {
        if attempts >= MAX_ATTEMPTS {
            println!("Game over! You've used all {} attempts.", MAX_ATTEMPTS);
            println!("The secret number was: {}", secret_number);
            break;
        }

        println!("Attempt {}/{}", attempts + 1, MAX_ATTEMPTS);
        println!("Please input your guess (1-100):");
        
        // 刷新输出缓冲区
        io::stdout().flush().unwrap();

        let mut guess = String::new();
        
        if io::stdin().read_line(&mut guess).is_err() {
            println!("Failed to read input.");
            continue;
        }
        
        // 限制输入长度
        if guess.trim().len() > MAX_INPUT_LENGTH {
            println!("Input too long! Maximum {} characters.", MAX_INPUT_LENGTH);
            continue;
        }
        
        // 解析并验证范围
        let guess: u32 = match guess.trim().parse() {
            Ok(num) if (1..=100).contains(&num) => num,
            Ok(_) => {
                println!("Please enter a number between 1 and 100.");
                continue;
            },
            Err(_) => {
                println!("Please enter a valid positive integer.");
                continue;
            },
        };

        attempts += 1;
        println!("You guessed: {}", guess);

        match guess.cmp(&secret_number) {
            Ordering::Less => println!("Too small!"),
            Ordering::Greater => println!("Too big!"),
            Ordering::Equal => { 
                println!("You win in {} attempts!", attempts);
                break;
            },
        }
    }
}

fn main() {

    guess_game_opt();

    println!("Guess the number!");

    let secret_number = rand::thread_rng().gen_range(1..=100);

    loop {
        println!("Please input your guess.");

        let mut guess = String::new();

        io::stdin().read_line(&mut guess).expect("Failed to read line");

        let guess: u32 = match guess.trim().parse() {
            Ok(num) => num,
            Err(_) => {
                println!("Please enter a valid number.");
                continue;
            },
        };

        println!("You guessed: {guess}");

        match guess.cmp(&secret_number) {
            Ordering::Less => println!("Too small!"),
            Ordering::Greater => println!("Too big!"),
            Ordering::Equal => { 
                println!("You win!");
                break;
            },
        }
    }
}
