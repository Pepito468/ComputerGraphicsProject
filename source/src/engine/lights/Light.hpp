#ifndef ENGINE_LIGHT_H
#define ENGINE_LIGHT_H

#include "Node3D.hpp"
#include "Types.hpp"
#include <glm/glm.hpp>

/// Base class for all light emitting nodes
class Light : public Node3D
{
    public:
        /// The color of the light
        glm::vec3 color = VEC3_ZERO;

        /// The intensity of the light
        BoundFloat radiance = PositiveFloat(0.0f);

        /// True if the node is currently emitting light; false otherwise
        bool isOn = true;

        /* Default constructor */
        Light() {}

        /* Default destructor */
        ~Light() override = default;

        /* Toggles the light */
        void toggle() {
            isOn = !isOn;
        }
};

/// Emits light in a sphere around the node
class PointLight : public Light
{
    public:
        BoundFloat radius = PositiveFloat(1.0f);
        BoundFloat decay = PositiveFloat(0.0f);

        /* Default constructor */
        PointLight() {}
        PointLight(const glm::vec3 position, const float radiance, const glm::vec3 color, const float radius, const float decay, const bool isOn = true) {
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

/// Emits a cone of light along the node's Z-axis
class SpotLight : public Light
{
    public:
        /// The half-angle of the spotlight's cone (alpha_in), in degrees
        BoundFloat aperture = BoundFloat(0.0f, 360.0f, 45.0f);

        /// The half-angle of the spotlight's decay cone (alpha_out), in degrees
        BoundFloat decay = BoundFloat(0.0f, 360.0f, 0.0f);

        /* Default constructor */
        SpotLight() {}
        SpotLight(const glm::vec3 position, const float radiance, const glm::vec3 color, const float aperture, const float decay, const bool isOn = true) {
            this->setLocalPosition(position);
            this->radiance = radiance;
            this->color = color;
            this->aperture = aperture;
            this->decay = decay;

            this->isOn = isOn;
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

/// Emits light in parallel rays along the node's Z-axis
class DirectionalLight : public Light {
    public:

    /*Default constructor*/
    DirectionalLight() : Light() {}

    DirectionalLight(const float radiance, const glm::vec3 color, const bool isOn = true) {
        this->radiance = radiance;
        this->color = color;

        this->isOn = isOn;
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
