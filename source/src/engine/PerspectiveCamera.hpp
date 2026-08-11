#ifndef ENGINE_PERSPECTIVECAMERA_H
#define ENGINE_PERSPECTIVECAMERA_H

#include "Camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>

class PerspectiveCamera : public Camera {

    public:

        float fov;
        float aspectRatio;


        PerspectiveCamera() : Camera() {
            this->fov = 90;
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

            glm::vec3 position = VEC3_ZERO;
            glm::vec3 rotation = VEC3_ZERO;
            glm::vec3 scale = VEC3_ONE;
            if (json.contains("position")) position = glm::vec3(json["position"][0].get<float>(), json["position"][1].get<float>(), json["position"][2].get<float>());
            if (json.contains("rotation")) rotation = glm::vec3(json["rotation"][0].get<float>(), json["rotation"][1].get<float>(), json["rotation"][2].get<float>());
            if (json.contains("scale")) scale = glm::vec3(json["scale"][0].get<float>(), json["scale"][1].get<float>(), json["scale"][2].get<float>());
            newNode->localPosition = position;
            newNode->localRotation = rotation;
            newNode->localScale = scale;
            newNode->localMatrix = newNode->computeLocalMatrixFromTransform(position, rotation, scale);
            newNode->commitUpdate();

            if (json.contains("nearValue")) newNode->nearValue = json["nearValue"].get<float>();
            if (json.contains("farValue")) newNode->farValue = json["farValue"].get<float>();
            if (json.contains("fov")) newNode->fov = json["fov"].get<float>();
            if (json.contains("aspectRatio")) newNode->aspectRatio = json["aspectRatio"].get<float>();

            return newNode;
        }
};

#endif
