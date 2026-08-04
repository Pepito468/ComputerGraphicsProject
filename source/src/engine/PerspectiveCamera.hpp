#ifndef ENGINE_PERSPECTIVECAMERA_H
#define ENGINE_PERSPECTIVECAMERA_H

#include "Camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>

class PerspectiveCamera : Camera {

    public:

        float fov;
        float aspectRatio;


        PerspectiveCamera() : Camera() {
            this->fov = 90;
            this->aspectRatio = 1.0f;
        }

        /** Alternative constructor with FOV and aspect ratio */
        PerspectiveCamera(const float near, const float far, const float fov, const float aspectRatio) {
            this->near = near;
            this->far = far;
            this->fov = fov;
            this->aspectRatio = aspectRatio;
        }

        virtual const glm::mat4 getProjectionMatrix() override {
            glm::mat4 p = glm::perspective(this->fov, this->aspectRatio, this->near, this->far);
            // Apply negation because the perspective function was made for OpenGL
            p[1][1] *= -1;
            return p;
        }

};

#endif
