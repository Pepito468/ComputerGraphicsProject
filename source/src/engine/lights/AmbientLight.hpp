#ifndef ENGINE_AMBIENTLIGHT_H
#define ENGINE_AMBIENTLIGHT_H

#include "Light.hpp"

class AmbientLight : public Light {

    public:

        glm::vec4 upper;
        glm::vec4 lower;
        glm::vec4 dir;

        AmbientLight() {}

        AmbientLight(glm::vec4 upper, glm::vec4 lower, glm::vec4 dir) {
            this->upper = upper;
            this->lower = lower;
            this->dir = dir;
        }

        AmbientLight(glm::vec3 upper, glm::vec3 lower, glm::vec3 dir) {
            this->upper = glm::vec4(upper, 0.0f);
            this->lower = glm::vec4(lower, 0.0f);
            this->dir = glm::vec4(dir, 0.0f);
        }
};

#endif
