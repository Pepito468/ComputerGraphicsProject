#ifndef LIGHT_H
#define LIGHT_H

#include "Node3D.hpp"
#include <string>

/// Base class for all light emitting nodes
abstract class Light : public Node3D
{
    public:
        /// The color of the light
        glm::vec3 color;

        /// The intensity of the light
        float radiance;

        /// If the node is currently emitting light
        bool isOn;

        Light() : Node3D(), color(glm::vec3(0f)), radiance(0f), isOn(true) {};

        void toggle()
        {
            isOn = !isOn;
        }
};

/// Emits light in a sphere around the node
class PointLight : public Light
{
    public:
        float radius;

    PointLight() : radius(0.0f) {}
};

/// Emits a cone of light along the node's Z-axis
class Spotlight : public Light
{
    public:
        /// The half-angle of the spotlight's cone, in radians
        float aperture;

        /// How far from the origin the light reaches
        float range;

        Spotlight() : aperture(0.0f), range(0.0f) {}
};

/// Emits light in parallel rays along the node's Z-axis
class DirectionalLight : public Light {};

#endif