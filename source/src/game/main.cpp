#include "Engine.hpp"
#include "Light.hpp"
#include "gamenodes/gamenode.hpp"
#include <iostream>
#include <ostream>

int main() {
    // Node Test
    Node *testnode = new GameNode();
    testnode->onEnter();
    std::cout << testnode->UUID << std::endl;
    testnode->onExit();
    std::cout << testnode->UUID << std::endl;

    Node *parent = new Light();
    Node *child = new Node();

    std::cout << parent->UUID << std::endl;
    std::cout << child->UUID << std::endl;

    parent->adopt(child);
    parent->adopt(child);
    parent->disown(child);
    parent->disown(child);


    delete testnode;
    delete parent;
    delete child;


    std::cout << "END TEST" << std::endl;


    Engine game;

    try {
        game.run(false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
