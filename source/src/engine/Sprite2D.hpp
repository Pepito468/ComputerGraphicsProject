#ifndef ENGINE_SPRITE2D_H
#define ENGINE_SPRITE2D_H

#include "Node2D.hpp"
#include <string>

class Sprite2D : public Node2D {

    public:

        std::string image;

        Sprite2D() {
            this->image = nullptr;
        }

        Sprite2D(const std::string imageName) {
            this->image = imageName;
        }

};

#endif
