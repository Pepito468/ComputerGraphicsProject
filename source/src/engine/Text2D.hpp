#ifndef ENGINE_TEXT2D_H
#define ENGINE_TEXT2D_H

#include "Node2D.hpp"
#include <string>

class Text2D : public Node2D {

    public:

        std::string text;

        Text2D() {
            this->text = "";
        }

        Text2D(const std::string text) {
            this->text = text;
        }

};

#endif
