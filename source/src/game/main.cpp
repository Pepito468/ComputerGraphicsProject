#include "Engine.hpp"
#include "gamenodes/gamenode.hpp"
#include <iostream>

int main() {
    // Node Test
    GameNode *node = new GameNode();
    node->onEnter();
    std::cout << node->UUID;
    node->onExit();
    std::cout << node->UUID;
    delete node;


    Engine game;

    try {
        game.run(false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
