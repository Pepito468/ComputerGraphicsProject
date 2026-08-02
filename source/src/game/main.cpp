#include "Engine.hpp"
#include "Light.hpp"
#include "gamenodes/gamenode.hpp"
#include <iostream>
#include <ostream>

int main() {
    // Node Test
    Node *testNode = new GameNode();
    testNode->onEnter();
    std::cout << "testNode UUID after onEnter(): " << testNode->UUID << std::endl;
    testNode->onExit();
    std::cout << "testNode UUID after onExit(): " << testNode->UUID << std::endl;

    Node *parent = new Light();
    Node *child = new Node();

    std::cout << "parent UUID: " << parent->UUID << std::endl;
    std::cout << "child UUID: " << child->UUID << std::endl;

    parent->adopt(child);
    parent->adopt(child);
    parent->disown(child);
    parent->disown(child);


    delete testNode;
    delete parent;
    delete child;


    std::cout << "END TEST" << std::endl << std::endl;


    Engine game;

    try {
        game.run(false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
