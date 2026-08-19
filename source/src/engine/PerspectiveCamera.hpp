#ifndef ENGINE_PERSPECTIVECAMERA_H
#define ENGINE_PERSPECTIVECAMERA_H

#include "Camera.hpp"
#include "glm/trigonometric.hpp"
#include <glm/ext/matrix_clip_space.hpp>

class PerspectiveCamera : public Camera {

    public:

        float fov;
        float aspectRatio;


        PerspectiveCamera() : Camera() {
            this->fov = glm::radians(90.0f);
            this->aspectRatio = 1.0f;
        }

        /** Alternative constructor with FOV and aspect ratio */
        PerspectiveCamera(const float nearValue, const float farValue, const float fov, const float aspectRatio) {
            this->nearValue = nearValue;
            this->farValue = farValue;
            this->fov = fov;
            this->aspectRatio = aspectRatio;
        }

        /** Constructor with the standard boundaries */
        PerspectiveCamera(const float nearValue, const float farValue, const float right, const float left, const float top, const float bottom) {
            this->nearValue = nearValue;
            this->farValue = farValue;
            this->aspectRatio = (right - left) / (top - bottom);
            this->fov = atan(top / nearValue) * 2;
        }

        virtual const glm::mat4 getProjectionMatrix() override {
            glm::mat4 p = glm::perspective(this->fov, this->aspectRatio, this->nearValue, this->farValue);
            // Apply negation because the perspective function was made for OpenGL
            p[1][1] *= -1;
            return p;
        }

        static PerspectiveCamera* fromJSON(const nlohmann::json& json) {
            PerspectiveCamera *newNode = new PerspectiveCamera();

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
            if (json.contains("fov")) newNode->fov = json["fov"].get<float>();
            if (json.contains("aspectRatio")) newNode->aspectRatio = json["aspectRatio"].get<float>();

            return newNode;
        }
};

#endif
