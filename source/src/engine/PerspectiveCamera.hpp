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

};

#endif
