#ifndef ENGINE_TEXT2D_H
#define ENGINE_TEXT2D_H

#include "Node2D.hpp"
#include "common.h"
#include <string>

class Text2D : public Node2D {

    public:

        inline static int incrementalID = 0.0f;

        std::string text;
        // automatically assigned
        int textID = 0.0f;
        // font
        std::string fontFace;
        bool isBold;
        bool isItalic;
        bool isSmall;
        TextAlignment alignment;
        TextRegistrationH regH;
        TextRegistrationV regV;
        glm::vec4 color;
        // stroke color
        glm::vec4 stroke;
        // shadow color
        glm::vec4 shadow;

        Text2D() {
            this->textID = incrementalID++;
            this->text = "";
        }

        Text2D(const std::string text,
                const glm::vec2 pos = {0, 0},
                std::string fontFace = "SS",
                const bool bold = false,
                const bool italic = false,
                const bool small = false,
                TextAlignment alignment = TAL_CENTER,
                TextRegistrationH regH = TRH_CENTER,
                TextRegistrationV regV = TRV_MIDDLE,
                const glm::vec4 color = {1, 1, 1, 1},
                const glm::vec4 stroke = {0, 0, 0, 1},
                const glm::vec4 shadow = {0, 0, 0, 0}) :
        Text2D() {

            this->text = text;
            this->setGlobalPosition(pos);
            if (fontFace != "SS" && fontFace != "SR")
                fontFace = "SS";
            else
                this->fontFace = fontFace;
            this->isBold = bold;
            this->isItalic = italic;
            this->isSmall = small;
            this->alignment = alignment;
            this->regH = regH;
            this->regV = regV;
            this->color = color;
            this->stroke = stroke;
            this->shadow = shadow;
        }

        static Text2D* fromJSON(const nlohmann::json& json, Text2D* node = nullptr) {
            Text2D *newNode = node ? node: new Text2D();

            Node2D::fromJSON(json, newNode);

            if (json.contains("text")) newNode->text = json["text"].get<std::string>();

            return newNode;
        }
};

#endif
