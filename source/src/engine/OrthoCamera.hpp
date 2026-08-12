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

            glm::vec3 position = VEC3_ZERO;
            glm::vec3 rotation = VEC3_ZERO;
            glm::vec3 scale = VEC3_ONE;
            if (json.contains("position")) position = glm::vec3(json["position"][0].get<float>(), json["position"][1].get<float>(), json["position"][2].get<float>());
            if (json.contains("rotation")) rotation = glm::vec3(json["rotation"][0].get<float>(), json["rotation"][1].get<float>(), json["rotation"][2].get<float>());
            if (json.contains("scale")) scale = glm::vec3(json["scale"][0].get<float>(), json["scale"][1].get<float>(), json["scale"][2].get<float>());
            newNode->localPosition = position;
            newNode->localRotation = rotation;
            newNode->localScale = scale;
            newNode->localMatrix = newNode->computeMatrixFromTransform(position, rotation, scale);
            newNode->commitLocalUpdate();

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
