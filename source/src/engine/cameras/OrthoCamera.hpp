#ifndef ENGINE_ORTHOCAMERA_H
#define ENGINE_ORTHOCAMERA_H

#include "Camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>

class OrthoCamera : public Camera
{
    public:
        /// Camera lower bound
        BoundFloat bottomBound = NegativeFloat(-5.0f);

        /// Camera higher bound
        BoundFloat topBound = PositiveFloat(5.0f);

        /// Camera leftmost bound
        BoundFloat leftBound = NegativeFloat(-4.0f);

        /// Camera rightmost bound
        BoundFloat rightBound = PositiveFloat(4.0f);;

        OrthoCamera() {};
        OrthoCamera(const float leftBound, const float rightBound, const float bottomBound, const float topBound, const float nearPlane, const float farPlane) {
            this->nearPlane = nearPlane;
            this->farPlane = farPlane;
            this->bottomBound = bottomBound;
            this->topBound = topBound;
            this->rightBound = rightBound;
            this->leftBound = leftBound;
        }

        const glm::mat4 getProjectionMatrix() override {
            // Apply reflection because the ortho function was made for OpenGL
            return
                glm::scale(MAT4_I, glm::vec3(1, -1, 1)) *
                glm::ortho<float>(this->leftBound, this->rightBound, this->bottomBound, this->topBound, this->nearPlane, this->farPlane);
        }

        static OrthoCamera* fromJSON(const nlohmann::json& json, OrthoCamera* node = nullptr) {
            OrthoCamera *newNode = node ? node: new OrthoCamera();

            Node3D::fromJSON(json, newNode);

            if (json.contains("nearValue")) newNode->nearPlane = json["nearValue"].get<float>();
            if (json.contains("farValue")) newNode->farPlane = json["farValue"].get<float>();
            if (json.contains("bottomValue")) newNode->bottomBound = json["bottomValue"].get<float>();
            if (json.contains("topValue")) newNode->topBound = json["topValue"].get<float>();
            if (json.contains("rightValue")) newNode->rightBound = json["rightValue"].get<float>();
            if (json.contains("leftValue")) newNode->leftBound = json["leftValue"].get<float>();

            return newNode;
        }
};

#endif
