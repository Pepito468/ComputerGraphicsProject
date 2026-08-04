#ifndef ENGINE_ORTHOCAMERA_H
#define ENGINE_ORTHOCAMERA_H

#include "Camera.hpp"
#include "glm/ext/matrix_clip_space.hpp"

class OrthoCamera : Camera {

    public:
        /// Camera lower bound
        float bottom;

        /// Camera higher bound
        float top;

        /// Camera leftmost bound
        float left;

        /// Camera rightmost bound
        float right;



        OrthoCamera(const float near, const float far, const float bottom, const float top, const float right, const float left) {
            this->near = near;
            this->far = far;
            this->bottom = bottom;
            this->top = top;
            this->right = right;
            this->left = left;
        }

        virtual const glm::mat4 getProjectionMatrix() override {
            // Apply scale because the ortho function was made for OpenGL
            return
                glm::scale(MAT4_I, glm::vec3(1, -1, 1)) *
                glm::ortho(this->left, this->right, this->bottom, this->top, this->near, this->far);
        }

};

#endif
