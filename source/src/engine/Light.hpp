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
            isOn = false;
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
        float decay;


        /* Default constructor */
        PointLight() {
            radius = 0.0f;
        }

        PointLight(glm::vec3 position, float radiance, glm::vec3 color, float radius, float decay) {
            this->position = position;
            this->radiance = radiance;
            this->color = color;
            this->radius = radius;
            this->decay = decay;

            this->isOn = true;
        }
};

/// Emits a cone of light along the node's Z-axis
class Spotlight : public Light {

    public:

        /// The half-angle of the spotlight's cone (alpha_in), in degrees
        float aperture;

        /// The half-angle of the spotlight's decay cone (alpha_out), in degrees
        float decay;

        /// Where is pointing the light
        glm::vec3 direction;


        /* Default constructor */
        Spotlight() {
            aperture = 0.0f;
            decay = 0.0f;
        }

        Spotlight(glm::vec3 position, float radiance, glm::vec3 color, float aperture, float decay, glm::vec3 direction) {
            this->position = position;
            this->radiance = radiance;
            this->color = color;
            this->aperture = aperture;
            this->decay = decay;
            this->direction = direction;

            this->isOn = true;
        }
};

/// Emits light in parallel rays along the node's Z-axis
class DirectionalLight : public Light {
    public:

    ///Where is pointing the light
    glm::vec3 direction;

    /*Default constructor*/
    DirectionalLight() : Light() {}

    DirectionalLight(float radiance, glm::vec3 color, glm::vec3 direction) {
        this->radiance = radiance;
        this->color = color;
        this->direction = direction;

        this->isOn = true;
    }
};

#endif
