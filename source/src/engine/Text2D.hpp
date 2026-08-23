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

        static Text2D* fromJSON(const nlohmann::json& json, Text2D* node = nullptr) {
            Text2D *newNode = node ? node: new Text2D();

            Node2D::fromJSON(json, newNode);

            if (json.contains("text")) newNode->text = json["text"].get<std::string>();

            return newNode;
        }
};

#endif
