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

            glm::vec2 globalPosition = VEC2_ZERO;
            float globalRotation = 0.0f;
            glm::vec2 globalScale = VEC2_ONE;
            if (json.contains("globalPosition")) globalPosition = glm::vec2(json["globalPosition"][0].get<float>(), json["globalPosition"][1].get<float>());
            if (json.contains("globalRotation")) globalRotation = json["globalRotation"].get<float>();
            if (json.contains("globalScale")) globalScale = glm::vec2(json["globalScale"][0].get<float>(), json["globalScale"][1].get<float>());
            newNode->globalPosition = globalPosition;
            newNode->globalRotation = globalRotation;
            newNode->globalScale = globalScale;
            newNode->globalMatrix = newNode->computeMatrixFromTransform(globalPosition, globalRotation, globalScale);
            newNode->commitGlobalUpdate();

            if (json.contains("image")) newNode->image = json["image"].get<std::string>();

            return newNode;
        }

};

#endif
