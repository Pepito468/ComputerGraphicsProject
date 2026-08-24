#ifndef ENGINE_SPOTLIGHT_H
#define ENGINE_SPOTLIGHT_H

#include "Light.hpp"
#include "Types.hpp"
#include "glm/trigonometric.hpp"
/// Emits a cone of light along the node's Z-axis
class SpotLight : public Light {

    public:

        /// The half-angle of the spotlight's cone (alpha_in), in degrees
        BoundFloat aperture = PositiveFloat(0.0f);

        /// The half-angle of the spotlight's decay cone (alpha_out), in degrees
        BoundFloat decay = PositiveFloat(0.0f);

        /* Default constructor */
        SpotLight() {
            aperture = glm::radians(45.0f);
            decay = 2.0f;
        }

        SpotLight(glm::vec3 position, float radiance, glm::vec3 color, float aperture, float decay, glm::vec3 direction, bool isOn = true) {
            this->setLocalPosition(position);
            this->radiance = radiance;
            this->color = color;
            this->aperture = aperture;
            this->decay = decay;
            this->setGlobalRotation(direction);
            this->isOn = isOn;
        }

        /** Returns the light's direction */
        const glm::vec3 getDicrection() const {
            // Note that the negative Z is considered the direction
            return this->getGlobalRotation();
        }

        static SpotLight* fromJSON(const nlohmann::json& json, SpotLight* node = nullptr) {
            SpotLight *newNode = node ? node: new SpotLight();

            Node3D::fromJSON(json, newNode);

            if (json.contains("color")) newNode->color = glm::vec3(json["color"][0].get<float>(), json["color"][1].get<float>(), json["color"][2].get<float>());
            if (json.contains("radiance")) newNode->radiance = json["radiance"].get<float>();
            if (json.contains("isOn")) newNode->isOn = json["radiance"].get<bool>();

            if (json.contains("aperture")) newNode->aperture = json["aperture"].get<float>();
            if (json.contains("decay")) newNode->decay = json["decay"].get<float>();

            return newNode;
        }
};

#endif
