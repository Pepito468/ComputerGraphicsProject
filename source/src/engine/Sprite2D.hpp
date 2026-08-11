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


        static Sprite2D* fromJSON(const nlohmann::json& json) {
            Sprite2D *newNode = new Sprite2D();

            if (json.contains("name")) newNode->name = json["name"].get<std::string>();

            glm::vec2 position = VEC2_ZERO;
            float rotation = 0.0f;
            glm::vec2 scale = VEC2_ONE;
            if (json.contains("position")) position = glm::vec2(json["position"][0].get<float>(), json["position"][1].get<float>());
            if (json.contains("rotation")) rotation = json["rotation"].get<float>();
            if (json.contains("scale")) scale = glm::vec2(json["scale"][0].get<float>(), json["scale"][1].get<float>());
            newNode->localPosition = position;
            newNode->localRotation = rotation;
            newNode->localScale = scale;
            newNode->localMatrix = newNode->computeLocalMatrixFromTransform(position, rotation, scale);
            newNode->commitUpdate();

            if (json.contains("image")) newNode->image = json["image"].get<std::string>();

            return newNode;
        }

};

#endif
