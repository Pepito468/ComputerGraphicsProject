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

            glm::vec2 localPosition = VEC2_ZERO;
            float localRotation = 0.0f;
            glm::vec2 localScale = VEC2_ONE;
            if (json.contains("position")) localPosition = glm::vec2(json["localPosition"][0].get<float>(), json["localPosition"][1].get<float>());
            if (json.contains("localRotation")) localRotation = json["localRotation"].get<float>();
            if (json.contains("localScale")) localScale = glm::vec2(json["localScale"][0].get<float>(), json["localScale"][1].get<float>());
            newNode->localPosition = localPosition;
            newNode->localRotation = localRotation;
            newNode->localScale = localScale;
            newNode->localMatrix = newNode->computeMatrixFromTransform(localPosition, localRotation, localScale);
            newNode->commitLocalUpdate();

            if (json.contains("image")) newNode->image = json["image"].get<std::string>();

            return newNode;
        }

};

#endif
