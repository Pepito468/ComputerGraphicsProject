/// Camera
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "Node3D.hpp"
#define GLM_FORCE_RADIANS
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

    protected:

        /// The distance of the near plane
        float nearPlane;

        /// The distance of the far plane
        float farPlane;


    public:

        /** Default constructor */
        Camera() {
            this->nearPlane = 0.1f;
            this->farPlane = 100.0f;
        }

        Camera(float nearPlane, float farPlane) {
            this->nearPlane = nearPlane;
            this->farPlane = farPlane;
        }

        /** Returns the projection matrix of this camera */
        virtual const glm::mat4 getProjectionMatrix() = 0;

        float getNearPlane() const {
            return this->nearPlane;
        }

        float getFarPlane() const {
            return this->farPlane;
        }

        const void setNearPlane(float nearPlane) {
            this->nearPlane = nearPlane;
        }

        const void setFarPlane(float farPlane) {
            this->farPlane = farPlane;
        }

        /** Returns the view matrix */
        const glm::mat4 getViewMatrix() {
            return glm::inverse(this->getGlobalMatrix());
        }

        /** Returns the direction that the camera is looking at */
        const glm::vec3 getLookingDirection() const {
            return -1.0f * this->getZAxis();
        }

};
#endif
