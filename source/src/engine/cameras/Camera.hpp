/// Camera
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "Node3D.hpp"
#include "Types.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/matrix.hpp>

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
        BoundFloat nearPlane = PositiveFloat(0.1f);

        /// The distance of the far plane
        BoundFloat farPlane = PositiveFloat(100.0f);

        /** Default constructor */
        Camera() {}
        Camera(const float nearPlane, const float farPlane) {
            this->nearPlane = nearPlane;
            this->farPlane = farPlane;
        }

        /** Returns the projection matrix of this camera */
        virtual const glm::mat4 getProjectionMatrix() = 0;

        /** Returns the view matrix */
        glm::mat4 getViewMatrix() const {
            return glm::inverse(this->getGlobalMatrix());
        }

        /** Returns the direction that the camera is looking at */
        glm::vec3 getLookingDirection() const {
            return -1.0f * this->getZAxis();
        }
};
#endif
