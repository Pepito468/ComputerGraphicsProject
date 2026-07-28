// Node superclass

#include <string>

class Node {
    public:
        // Unique ID for every Node
        long long int UUID;

        // Node Name
        std::string Name;

        // Called by the Engine when the node is added to the scene
        virtual void onEnter() {}

        // Called by the Engine when the node is removed from the scene
        virtual void onExit() {}
};
