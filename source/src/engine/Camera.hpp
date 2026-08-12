/// Camera
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "Node3D.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/matrix.hpp"
#include <glm/ext/matrix_clip_space.hpp>
/*
 *  TODO:
 *  global OK
 *  view (view matrix is the inverse of the camera global matrix) OK
 *  projection (orthogonal, use its matrix) OK
 *  normalization (for every point)
 *  screen (pixel coordinates)
 *  Engine does the last 2?
 * */

/// A camera in the scene
class Camera : public Node3D
{

    public:

        /// The distance of the near plane
        float nearValue;

        /// The distance of the far plane
        float farValue;



        /** Default constructor */
        Camera() {
            this->nearValue = 0.1f;
            this->farValue = 100.0f;
        }

        /**
         * Returns the projection matrix of this camera
         * */
        virtual const glm::mat4 getProjectionMatrix() = 0;

        /**
         * Returns the view matrix
         * */
        const glm::mat4 getViewMatrix() const {
            return glm::inverse(this->globalMatrix);
        }

};
#endif
