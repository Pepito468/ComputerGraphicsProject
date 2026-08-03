#include "Engine.hpp"
#include "gamenodes/gamenode.hpp"
#include <iostream>
#include <ostream>

int main() {
    Engine game;

    try {
        game.run(false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
