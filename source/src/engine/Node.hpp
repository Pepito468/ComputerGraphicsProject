// Node superclass
#ifndef ENGINE_NODE_H
#define ENGINE_NODE_H

#include <list>
#include <string>

class Node {
    public:
        // Unique ID for every Node
        long long int UUID;

        // Node Name
        std::string name;

        // Node's children
        std::list<Node> children;

        // Called by the Engine when the node is added to the scene
        virtual void onEnter() {}

        // Called by the Engine when the node is removed from the scene
        virtual void onExit() {}
};

#endif
