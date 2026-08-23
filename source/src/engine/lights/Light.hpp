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

/// Emits a cone of light along the node's Z-axis
class SpotLight : public Light {

    public:

        /// The half-angle of the spotlight's cone (alpha_in), in degrees
        float aperture;

        /// The half-angle of the spotlight's decay cone (alpha_out), in degrees
        float decay;

        /// Where is pointing the light
        glm::vec3 direction;


        /* Default constructor */
        SpotLight() {
            aperture = 0.0f;
            decay = 0.0f;
        }

        SpotLight(glm::vec3 position, float radiance, glm::vec3 color, float aperture, float decay, glm::vec3 direction, bool isOn = true) {
            this->setLocalPosition(position);
            this->radiance = radiance;
            this->color = color;
            this->aperture = aperture;
            this->decay = decay;
            this->direction = direction;

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

    ///Where is pointing the light
    glm::vec3 direction;

    /*Default constructor*/
    DirectionalLight() : Light() {}

    DirectionalLight(float radiance, glm::vec3 color, glm::vec3 direction, bool isOn = true) {
        this->radiance = radiance;
        this->color = color;
        this->direction = direction;

        this->isOn = true;
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

struct AmbientLight {
    glm::vec4 upper;
    glm::vec4 lower;
    glm::vec4 dir;
};


#endif
