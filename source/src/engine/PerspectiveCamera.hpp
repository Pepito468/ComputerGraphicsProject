#ifndef ENGINE_PERSPECTIVECAMERA_H
#define ENGINE_PERSPECTIVECAMERA_H

#include "Camera.hpp"
#include "glm/trigonometric.hpp"
#include <glm/ext/matrix_clip_space.hpp>

class PerspectiveCamera : public Camera {

    protected:

        float fov;
        float aspectRatio;

    public:

        /** Default constructor */
        PerspectiveCamera() : Camera() {
            this->fov = glm::radians(90.0f);
            this->aspectRatio = 1.0f;
        }

        /** Alternative constructor with FOV and aspect ratio */
        PerspectiveCamera(const float nearPlane, const float farPlane, const float fov, const float aspectRatio) {
            this->nearPlane = nearPlane;
            this->farPlane = farPlane;
            this->fov = fov;
            this->aspectRatio = aspectRatio;
        }

        /** Constructor with the standard boundaries */
        PerspectiveCamera(const float nearValue, const float farValue, const float rightBound, const float leftBound, const float topBound, const float bottomBound) {
            this->nearPlane = nearValue;
            this->farPlane = farValue;
            this->aspectRatio = (rightBound - leftBound) / (topBound - bottomBound);
            this->fov = atan(topBound / nearValue) * 2;
        }

        float getFOV() const {
            return this->fov;
        }

        float getAspectRatio() const {
            return this->aspectRatio;
        }

        const void setFOV(const float FOV) {
            this->fov = FOV;
        }

        const void setAspectRatio(const float aspectRatio) {
            this->aspectRatio = aspectRatio;
        }

        virtual const glm::mat4 getProjectionMatrix() override {
            glm::mat4 p = glm::perspective(this->fov, this->aspectRatio, this->nearPlane, this->farPlane);
            // Apply manual reflection because the perspective function was made for OpenGL
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

            if (json.contains("nearValue")) newNode->nearPlane = json["nearValue"].get<float>();
            if (json.contains("farValue")) newNode->farPlane = json["farValue"].get<float>();
            if (json.contains("fov")) newNode->fov = json["fov"].get<float>();
            if (json.contains("aspectRatio")) newNode->aspectRatio = json["aspectRatio"].get<float>();

            return newNode;
        }
};

#endif
