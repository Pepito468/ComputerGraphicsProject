#ifndef ENGINE_SPRITE2D_H
#define ENGINE_SPRITE2D_H

#include "../Node2D.hpp"
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


        static Sprite2D* fromJSON(const nlohmann::json& json, Sprite2D* node = nullptr) {
            Sprite2D *newNode = node ? node: new Sprite2D();

            Node2D::fromJSON(json, newNode);

            if (json.contains("image")) newNode->image = json["image"].get<std::string>();

            return newNode;
        }

};

#endif
