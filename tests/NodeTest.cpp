#include "Node.hpp"
#include "Light.hpp"
#include <iostream>

int main() {
    info("STARTING NODE TEST");

    Node *parent = new Light();
    Node *child = new Node();

    // No collision
    assert(parent->UUID != child->UUID);
    std::cout << "parent UUID: " << parent->UUID << std::endl;
    std::cout << "child UUID: " << child->UUID << std::endl;

    // Add child
    parent->adopt(child);
    assert(parent->children.contains(child));

    // Should throw a warning
    parent->adopt(child);

    // Remove child
    parent->disown(child);
    assert(!parent->children.contains(child));

    // Should throw a warning
    parent->disown(child);

    delete parent;
    delete child;

    info("END OF NODE TEST");
    return 0;
}
