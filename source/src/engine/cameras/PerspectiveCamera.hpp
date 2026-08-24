#ifndef ENGINE_PERSPECTIVECAMERA_H
#define ENGINE_PERSPECTIVECAMERA_H

#include "Camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include "Types.hpp"

class PerspectiveCamera : public Camera {

    protected:

        BoundFloat fov = BoundFloat(0.0f, glm::radians(130.0f), glm::radians(10.0f)); // TODO: maybe constraints can be put into the user-made logic instead of here
        BoundFloat aspectRatio = PositiveFloat(0.0f);

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

        void setFOV(const float FOV) {
            this->fov = FOV;
        }

        void setAspectRatio(const float aspectRatio) {
            this->aspectRatio = aspectRatio;
        }

        virtual const glm::mat4 getProjectionMatrix() override {
            glm::mat4 p = glm::perspective((float)this->fov, (float)this->aspectRatio, (float)this->nearPlane, (float)this->farPlane);
            // Apply manual reflection because the perspective function was made for OpenGL
            p[1][1] *= -1;
            return p;
        }

        static PerspectiveCamera* fromJSON(const nlohmann::json& json, PerspectiveCamera* node = nullptr) {
            PerspectiveCamera *newNode = node ? node: new PerspectiveCamera();

            Node3D::fromJSON(json, newNode);

            if (json.contains("nearValue")) newNode->nearPlane = json["nearValue"].get<float>();
            if (json.contains("farValue")) newNode->farPlane = json["farValue"].get<float>();
            if (json.contains("fov")) newNode->fov = json["fov"].get<float>();
            if (json.contains("aspectRatio")) newNode->aspectRatio = json["aspectRatio"].get<float>();

            return newNode;
        }
};

#endif
