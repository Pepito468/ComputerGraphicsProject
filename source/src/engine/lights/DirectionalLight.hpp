#ifndef ENGINE_DIRECTIONALLIGHT_H
#define ENGINE_DIRECTIONALLIGHT_H

#include "Light.hpp"
#include <glm/ext/vector_float3.hpp>

/// Emits light in parallel rays along the node's Z-axis
class DirectionalLight : public Light {

    public:

        /*Default constructor*/
        DirectionalLight() : Light() {}

        DirectionalLight(float radiance, glm::vec3 color, glm::vec3 direction, bool isOn = true) {
            this->radiance = radiance;
            this->color = color;
            this->setGlobalRotation(direction);
            this->isOn = isOn;
        }

        /** Returns the light's direction */
        const glm::vec3 getDicrection() const {
            // Note that the negative Z is considered the direction
            return this->getGlobalRotation();
        }

        static DirectionalLight* fromJSON(const nlohmann::json& json, DirectionalLight* node = nullptr) {
            DirectionalLight *newNode = node ? node: new DirectionalLight();

            Node3D::fromJSON(json, newNode);

            if (json.contains("color")) newNode->color = glm::vec3(json["color"][0].get<float>(), json["color"][1].get<float>(), json["color"][2].get<float>());
            if (json.contains("radiance")) newNode->radiance = json["radiance"].get<float>();
            if (json.contains("isOn")) newNode->isOn = json["radiance"].get<bool>();

            return newNode;
        }
};

#endif
