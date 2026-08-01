#ifndef ENGINE_LIGHT_H
#define ENGINE_LIGHT_H

#include "Node3D.hpp"

/// Base class for all light emitting nodes
class Light : public Node3D {

    public:

        /// The color of the light
        glm::vec3 color;

        /// The intensity of the light
        float radiance;

        /// True if the node is currently emitting light; false otherwise
        bool isOn;



        /* Default constructor */
        Light() {
            color = glm::vec3(0.0f);
            radiance = 0.0f;
            isOn = true;
        }

        /* Default virtual destructor */
        virtual ~Light() = default;

        /* Toggles the light */
        void toggle() {
            isOn = !isOn;
        }
};

/// Emits light in a sphere around the node
class PointLight : public Light {

    public:

        float radius;


        /* Default constructor */
        PointLight() {
            radius = 0.0f;
        }
};

/// Emits a cone of light along the node's Z-axis
class Spotlight : public Light {

    public:

        /// The half-angle of the spotlight's cone, in radians
        float aperture;

        /// How far from the origin the light reaches
        float range;


        /* Default constructor */
        Spotlight() {
            aperture = 0.0f;
            range = 0.0f;
        }
};

/// Emits light in parallel rays along the node's Z-axis
class DirectionalLight : public Light {};

#endif
