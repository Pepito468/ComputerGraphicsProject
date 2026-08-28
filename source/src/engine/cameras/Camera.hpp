/// Camera
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "Node3D.hpp"
#include "Types.hpp"

/// A camera in the scene
class Camera : public Node3D
{

    protected:

        /// The distance of the near plane
        BoundFloat nearPlane = PositiveFloat(0.0f);

        /// The distance of the far plane
        BoundFloat farPlane = PositiveFloat(0.0f);

        /// Hints the Engine that this camera wants to be main
        bool isMain;


    public:

        /** Default constructor */
        Camera() {
            this->nearPlane = 0.1f;
            this->farPlane = 100.0f;
            this->isMain = false;
        }

        Camera(const float nearPlane, const float farPlane, const bool isMain = false) {
            this->nearPlane = nearPlane;
            this->farPlane = farPlane;
            this->isMain = isMain;
        }

        /** Returns the projection matrix of this camera */
        virtual const glm::mat4 getProjectionMatrix() = 0;

        float getNearPlane() const {
            return this->nearPlane;
        }

        float getFarPlane() const {
            return this->farPlane;
        }

        bool getIsMain() const {
            return this->isMain;
        }

        void setNearPlane(float nearPlane) {
            this->nearPlane = nearPlane;
        }

        void setFarPlane(float farPlane) {
            this->farPlane = farPlane;
        }

        void setMain() {
            this->isMain = true;
        }

        void setNotMain() {
            this->isMain = false;
        }

        /** Returns the view matrix */
        const glm::mat4 getViewMatrix() const {
            return glm::inverse(this->getGlobalMatrix());
        }

        /** Returns the direction that the camera is looking at */
        const glm::vec3 getLookingDirection() const {
            return -1.0f * this->getZAxis();
        }
};
#endif
