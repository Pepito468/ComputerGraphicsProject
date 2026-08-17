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

        static Text2D* fromJSON(const nlohmann::json& json) {
            Text2D *newNode = new Text2D();

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

            if (json.contains("text")) newNode->text = json["text"].get<std::string>();

            return newNode;
        }
};

#endif
