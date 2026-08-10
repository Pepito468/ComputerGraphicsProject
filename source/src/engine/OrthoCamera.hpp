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

};

#endif
