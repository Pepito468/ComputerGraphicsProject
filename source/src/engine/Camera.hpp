#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "Node3D.hpp"

/// A camera in the scene
class Camera : public Node3D
{
    public:

        /// The camera's field of view
        float fov;

        /// The distance of the near plane
        float near;

        /// The distance of the far plane
        float far;

        /// The camera's aspect ratio
        float aspect;

        enum ProjectionType
        {
            ORTHOGRAPHIC,
            PERSPECTIVE
        };

        /// The projection the camera should use
        ProjectionType projection;



        /* Default constructor */
        Camera() {
            fov = 0.0f;
            near = 0.0f;
            far = 0.0f;
            aspect = 0.0f;
            projection = ORTHOGRAPHIC;
        }

        /// Returns the View-Projection matrix of this camera
        glm::mat4 calculateViewProjection() const
        {
            //TODO
        }
};
#endif
