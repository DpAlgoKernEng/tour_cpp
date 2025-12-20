mod core;

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
    select_game!(2);
}