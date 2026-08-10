#ifndef ENGINE_UPDATENODE3D_H
#define ENGINE_UPDATENODE3D_H

#include "Node3D.hpp"

class UpdateNode: public Node3D {

    public:

        /** This method will be called every frame by the Engine */
        void (*update)(UpdateNode&) = nullptr;

};

#endif
