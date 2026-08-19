#ifndef ENGINE_ORTHOCAMERA_H
#define ENGINE_ORTHOCAMERA_H

#include "Camera.hpp"
#include "glm/ext/matrix_clip_space.hpp"

class OrthoCamera : public Camera {

    public:
        /// Camera lower bound
        float bottomValue;

        /// Camera higher bound
        float topValue;

        /// Camera leftmost bound
        float leftValue;

        /// Camera rightmost bound
        float rightValue;


        OrthoCamera() {
            this->bottomValue = -10.0f;
            this->topValue = 10.0f;
            this->rightValue = -10.0f;
            this->leftValue = 10.0f;
        };

        OrthoCamera(const float nearValue, const float farValue, const float bottomValue, const float topValue, const float rightValue, const float leftValue) {
            this->nearValue = nearValue;
            this->farValue = farValue;
            this->bottomValue = bottomValue;
            this->topValue = topValue;
            this->rightValue = rightValue;
            this->leftValue = leftValue;
        }

        virtual const glm::mat4 getProjectionMatrix() override {
            // Apply scale because the ortho function was made for OpenGL
            return
                glm::scale(MAT4_I, glm::vec3(1, -1, 1)) *
                glm::ortho(this->leftValue, this->rightValue, this->bottomValue, this->topValue, this->nearValue, this->farValue);
        }


        static OrthoCamera* fromJSON(const nlohmann::json& json) {
            OrthoCamera *newNode = new OrthoCamera();

            if (json.contains("name")) newNode->name = json["name"].get<std::string>();

            glm::vec3 globalPosition = VEC3_ZERO;
            glm::vec3 globalRotation = VEC3_ZERO;
            glm::vec3 globalScale = VEC3_ONE;
            if (json.contains("globalPosition")) globalPosition = glm::vec3(json["globalPosition"][0].get<float>(), json["globalPosition"][1].get<float>(), json["globalPosition"][2].get<float>());
            if (json.contains("globalRotation")) globalRotation = glm::vec3(json["globalRotation"][0].get<float>(), json["globalRotation"][1].get<float>(), json["globalRotation"][2].get<float>());
            if (json.contains("globalScale")) globalScale = glm::vec3(json["globalScale"][0].get<float>(), json["globalScale"][1].get<float>(), json["globalScale"][2].get<float>());
            newNode->setGlobalPosition(globalPosition);
            newNode->setGlobalRotation(globalRotation);
            newNode->setGlobalScale(globalScale);

            if (json.contains("nearValue")) newNode->nearValue = json["nearValue"].get<float>();
            if (json.contains("farValue")) newNode->farValue = json["farValue"].get<float>();
            if (json.contains("bottomValue")) newNode->bottomValue = json["bottomValue"].get<float>();
            if (json.contains("topValue")) newNode->topValue = json["topValue"].get<float>();
            if (json.contains("rightValue")) newNode->rightValue = json["rightValue"].get<float>();
            if (json.contains("leftValue")) newNode->leftValue = json["leftValue"].get<float>();

            return newNode;
        }
};

#endif
