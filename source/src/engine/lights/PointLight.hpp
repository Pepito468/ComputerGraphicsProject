#ifndef ENGINE_POINTLIGHT_H
#define ENGINE_POINTLIGHT_H

#include "Light.hpp"
#include "Types.hpp"

/// Emits light in a sphere around the node
class PointLight : public Light {

    public:

        /** Light radius */
        BoundFloat radius = PositiveFloat(0.0f);
        /** Light decay */
        float decay = PositiveFloat(0.0f);


        /* Default constructor */
        PointLight() {
            this->radius = 1.0f;
            this->decay = 2.0f;
        }

        PointLight(glm::vec3 position, float radiance, glm::vec3 color, float radius, float decay, bool isOn = true) {
            this->setLocalPosition(position);
            this->radiance = radiance;
            this->color = color;
            this->radius = radius;
            this->decay = decay;
            this->isOn = isOn;
        }

        static PointLight* fromJSON(const nlohmann::json& json, PointLight* node = nullptr) {
            PointLight *newNode = node ? node: new PointLight();

            Node3D::fromJSON(json, newNode);

            if (json.contains("radius")) newNode->radius = json["radius"].get<float>();
            if (json.contains("decay")) newNode->decay = json["decay"].get<float>();

            return newNode;
        }
};

#endif
