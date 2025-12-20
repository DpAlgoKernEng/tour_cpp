#include "guess_game_v1.h"
#include "guess_game_v2.h"
#include "guess_game_v3.h"

#define GUESS_GAME_V3

int main() {

#ifdef GUESS_GAME_V1
    start_guessing_game_v1();
#elif defined(GUESS_GAME_V2)
    start_guessing_game_v2();
#elif defined(GUESS_GAME_V3)
    start_guessing_game_v3();
#else
    #error "No guessing game version defined"
#endif
    
    return 0;
}