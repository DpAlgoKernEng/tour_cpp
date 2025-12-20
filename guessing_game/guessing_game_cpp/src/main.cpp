#include "guess_game_v1.hpp"
#include "guess_game_v2.hpp"

#define GUESS_GAME_V2

auto main() -> int {

#ifdef GUESS_GAME_V1
    start_guessing_game_v1();
#elif defined(GUESS_GAME_V2)
    GuessGame::start_guessing_game_v2();;
#else
    #error "No guessing game version defined"
#endif

    return 0;
}