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

        static PointLight* fromJSON(const nlohmann::json& json) {
            PointLight *newNode = new PointLight();

            if (json.contains("name")) newNode->name = json["name"].get<std::string>();

            glm::vec3 position = VEC3_ZERO;
            glm::vec3 rotation = VEC3_ZERO;
            glm::vec3 scale = VEC3_ONE;
            if (json.contains("position")) position = glm::vec3(json["position"][0].get<float>(), json["position"][1].get<float>(), json["position"][2].get<float>());
            if (json.contains("rotation")) rotation = glm::vec3(json["rotation"][0].get<float>(), json["rotation"][1].get<float>(), json["rotation"][2].get<float>());
            if (json.contains("scale")) scale = glm::vec3(json["scale"][0].get<float>(), json["scale"][1].get<float>(), json["scale"][2].get<float>());
            newNode->localPosition = position;
            newNode->localRotation = rotation;
            newNode->localScale = scale;
            newNode->localMatrix = newNode->computeLocalMatrixFromTransform(position, rotation, scale);
            newNode->commitUpdate();

            if (json.contains("color")) newNode->color = glm::vec3(json["color"][0].get<float>(), json["color"][1].get<float>(), json["color"][2].get<float>());
            if (json.contains("radiance")) newNode->radiance = json["radiance"].get<float>();
            if (json.contains("isOn")) newNode->isOn = json["radiance"].get<bool>();

            if (json.contains("radius")) newNode->radius = json["radius"].get<float>();

            return newNode;
        }
};

/// Emits a cone of light along the node's Z-axis
class SpotLight : public Light {

    public:

        /// The half-angle of the spotlight's cone (alpha_in), in radians
        float aperture;

        /// The half-angle of the spotlight's decay cone (alpha_out), in radians
        float decay;

        /// How far from the origin the light reaches
        float range;


        /* Default constructor */
        SpotLight() {
            aperture = 0.0f;
            decay = 0.0f;
            range = 0.0f;
        }

        static SpotLight* fromJSON(const nlohmann::json& json) {
            SpotLight *newNode = new SpotLight();

            if (json.contains("name")) newNode->name = json["name"].get<std::string>();

            glm::vec3 position = VEC3_ZERO;
            glm::vec3 rotation = VEC3_ZERO;
            glm::vec3 scale = VEC3_ONE;
            if (json.contains("position")) position = glm::vec3(json["position"][0].get<float>(), json["position"][1].get<float>(), json["position"][2].get<float>());
            if (json.contains("rotation")) rotation = glm::vec3(json["rotation"][0].get<float>(), json["rotation"][1].get<float>(), json["rotation"][2].get<float>());
            if (json.contains("scale")) scale = glm::vec3(json["scale"][0].get<float>(), json["scale"][1].get<float>(), json["scale"][2].get<float>());
            newNode->localPosition = position;
            newNode->localRotation = rotation;
            newNode->localScale = scale;
            newNode->localMatrix = newNode->computeLocalMatrixFromTransform(position, rotation, scale);
            newNode->commitUpdate();

            if (json.contains("color")) newNode->color = glm::vec3(json["color"][0].get<float>(), json["color"][1].get<float>(), json["color"][2].get<float>());
            if (json.contains("radiance")) newNode->radiance = json["radiance"].get<float>();
            if (json.contains("isOn")) newNode->isOn = json["radiance"].get<bool>();

            if (json.contains("aperture")) newNode->aperture = json["aperture"].get<float>();
            if (json.contains("decay")) newNode->decay = json["decay"].get<float>();
            if (json.contains("range")) newNode->range = json["range"].get<float>();

            return newNode;
        }
};

/// Emits light in parallel rays along the node's Z-axis
class DirectionalLight : public Light {

    public:

        static DirectionalLight* fromJSON(const nlohmann::json& json) {
            DirectionalLight *newNode = new DirectionalLight();

            if (json.contains("name")) newNode->name = json["name"].get<std::string>();

            glm::vec3 position = VEC3_ZERO;
            glm::vec3 rotation = VEC3_ZERO;
            glm::vec3 scale = VEC3_ONE;
            if (json.contains("position")) position = glm::vec3(json["position"][0].get<float>(), json["position"][1].get<float>(), json["position"][2].get<float>());
            if (json.contains("rotation")) rotation = glm::vec3(json["rotation"][0].get<float>(), json["rotation"][1].get<float>(), json["rotation"][2].get<float>());
            if (json.contains("scale")) scale = glm::vec3(json["scale"][0].get<float>(), json["scale"][1].get<float>(), json["scale"][2].get<float>());
            newNode->localPosition = position;
            newNode->localRotation = rotation;
            newNode->localScale = scale;
            newNode->localMatrix = newNode->computeLocalMatrixFromTransform(position, rotation, scale);
            newNode->commitUpdate();

            if (json.contains("color")) newNode->color = glm::vec3(json["color"][0].get<float>(), json["color"][1].get<float>(), json["color"][2].get<float>());
            if (json.contains("radiance")) newNode->radiance = json["radiance"].get<float>();
            if (json.contains("isOn")) newNode->isOn = json["radiance"].get<bool>();

            return newNode;
        }
};

#endif
