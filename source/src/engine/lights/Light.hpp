#ifndef ENGINE_LIGHT_H
#define ENGINE_LIGHT_H

#include "Node3D.hpp"
#include "Types.hpp"

/// Base class for all light emitting nodes
class Light : public Node3D {

    public:

        /// The color of the light
        glm::vec3 color;

        /// The intensity of the light
        BoundFloat radiance = PositiveFloat(0.0f);

        /// True if the node is currently emitting light; false otherwise
        bool isOn;



        /* Default constructor */
        Light() {
            color = glm::vec3(1.0f);
            radiance = 1.0f;
            isOn = true;
        }

        /* Default virtual destructor */
        virtual ~Light() = default;

        /* Toggles the light */
        void toggle() {
            isOn = !isOn;
        }
};

#endif
