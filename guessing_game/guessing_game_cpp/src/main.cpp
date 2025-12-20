#include <iostream>

auto main() -> int {
    int number_to_guess = 42; // The number the user has to guess
    int user_guess = 0;

    std::cout << "Welcome to the Guessing Game!" << std::endl;
    std::cout << "Try to guess the number between 1 and 100." << std::endl;

    while (user_guess != number_to_guess) {
        std::cout << "Enter your guess: ";
        std::cin >> user_guess;

        if (user_guess < number_to_guess) {
            std::cout << "Too low! Try again." << std::endl;
        } else if (user_guess > number_to_guess) {
            std::cout << "Too high! Try again." << std::endl;
        } else {
            std::cout << "Congratulations! You've guessed the number!" << std::endl;
        }
    }

    return 0;
}