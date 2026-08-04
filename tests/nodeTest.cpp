#include "Node.hpp"
#include "Light.hpp"
#include "Starter.hpp"
#include <iostream>

int main() {
    std::cout << "NODE TEST" << std::endl << std::endl;

    Node *parent = new Light();
    Node *child = new Node();

    std::cout << "parent UUID: " << parent->UUID << std::endl;
    std::cout << "child UUID: " << child->UUID << std::endl;

    parent->adopt(child);
    parent->adopt(child);
    parent->disown(child);
    parent->disown(child);

    delete parent;
    delete child;

    return 0;
}
