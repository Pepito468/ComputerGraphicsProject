// 3DNode
#ifndef ENGINE_NODE3D_H
#define ENGINE_NODE3D_H

#include "Node.hpp"
#include "Vector3.hpp"

class Node3D : public Node {

    public:
        // Node's position in the 3D Spaceù
        Vector3 position;

        // Node's rotation
        Vector3 rotation;

        // Node's scale
        Vector3 scale;

};

#endif
