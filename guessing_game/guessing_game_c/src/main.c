#include "guess_game_v1.h"
#include "guess_game_v2.h"

#define GUESS_GAME_V2

int main() {

#ifdef GUESS_GAME_V1
    start_guessing_game_v1();
#elif defined(GUESS_GAME_V2)
    start_guessing_game_v2();
#else
    #error "No guessing game version defined"
#endif
    
    return 0;
}