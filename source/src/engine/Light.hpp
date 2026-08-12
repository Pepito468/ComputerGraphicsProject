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

            glm::vec3 globalPosition = VEC3_ZERO;
            glm::vec3 globalRotation = VEC3_ZERO;
            glm::vec3 globalScale = VEC3_ONE;
            if (json.contains("globalPosition")) globalPosition = glm::vec3(json["globalPosition"][0].get<float>(), json["globalPosition"][1].get<float>(), json["globalPosition"][2].get<float>());
            if (json.contains("globalRotation")) globalRotation = glm::vec3(json["globalRotation"][0].get<float>(), json["globalRotation"][1].get<float>(), json["globalRotation"][2].get<float>());
            if (json.contains("globalScale")) globalScale = glm::vec3(json["globalScale"][0].get<float>(), json["globalScale"][1].get<float>(), json["globalScale"][2].get<float>());
            newNode->globalPosition = globalPosition;
            newNode->globalRotation = globalRotation;
            newNode->globalScale = globalScale;
            newNode->globalMatrix = newNode->computeMatrixFromTransform(globalPosition, globalRotation, globalScale);
            newNode->commitGlobalUpdate();

            glm::vec3 localPosition = VEC3_ZERO;
            glm::vec3 localRotation = VEC3_ZERO;
            glm::vec3 localScale = VEC3_ONE;
            if (json.contains("localPosition")) localPosition = glm::vec3(json["localPosition"][0].get<float>(), json["localPosition"][1].get<float>(), json["localPosition"][2].get<float>());
            if (json.contains("localRotation")) localRotation = glm::vec3(json["localRotation"][0].get<float>(), json["localRotation"][1].get<float>(), json["localRotation"][2].get<float>());
            if (json.contains("localScale")) localScale = glm::vec3(json["localScale"][0].get<float>(), json["localScale"][1].get<float>(), json["localScale"][2].get<float>());
            newNode->localPosition = localPosition;
            newNode->localRotation = localRotation;
            newNode->localScale = localScale;
            newNode->localMatrix = newNode->computeMatrixFromTransform(localPosition, localRotation, localScale);
            newNode->commitLocalUpdate();

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

            glm::vec3 globalPosition = VEC3_ZERO;
            glm::vec3 globalRotation = VEC3_ZERO;
            glm::vec3 globalScale = VEC3_ONE;
            if (json.contains("globalPosition")) globalPosition = glm::vec3(json["globalPosition"][0].get<float>(), json["globalPosition"][1].get<float>(), json["globalPosition"][2].get<float>());
            if (json.contains("globalRotation")) globalRotation = glm::vec3(json["globalRotation"][0].get<float>(), json["globalRotation"][1].get<float>(), json["globalRotation"][2].get<float>());
            if (json.contains("globalScale")) globalScale = glm::vec3(json["globalScale"][0].get<float>(), json["globalScale"][1].get<float>(), json["globalScale"][2].get<float>());
            newNode->globalPosition = globalPosition;
            newNode->globalRotation = globalRotation;
            newNode->globalScale = globalScale;
            newNode->globalMatrix = newNode->computeMatrixFromTransform(globalPosition, globalRotation, globalScale);
            newNode->commitGlobalUpdate();

            glm::vec3 localPosition = VEC3_ZERO;
            glm::vec3 localRotation = VEC3_ZERO;
            glm::vec3 localScale = VEC3_ONE;
            if (json.contains("localPosition")) localPosition = glm::vec3(json["localPosition"][0].get<float>(), json["localPosition"][1].get<float>(), json["localPosition"][2].get<float>());
            if (json.contains("localRotation")) localRotation = glm::vec3(json["localRotation"][0].get<float>(), json["localRotation"][1].get<float>(), json["localRotation"][2].get<float>());
            if (json.contains("localScale")) localScale = glm::vec3(json["localScale"][0].get<float>(), json["localScale"][1].get<float>(), json["localScale"][2].get<float>());
            newNode->localPosition = localPosition;
            newNode->localRotation = localRotation;
            newNode->localScale = localScale;
            newNode->localMatrix = newNode->computeMatrixFromTransform(localPosition, localRotation, localScale);
            newNode->commitLocalUpdate();

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

            glm::vec3 globalPosition = VEC3_ZERO;
            glm::vec3 globalRotation = VEC3_ZERO;
            glm::vec3 globalScale = VEC3_ONE;
            if (json.contains("globalPosition")) globalPosition = glm::vec3(json["globalPosition"][0].get<float>(), json["globalPosition"][1].get<float>(), json["globalPosition"][2].get<float>());
            if (json.contains("globalRotation")) globalRotation = glm::vec3(json["globalRotation"][0].get<float>(), json["globalRotation"][1].get<float>(), json["globalRotation"][2].get<float>());
            if (json.contains("globalScale")) globalScale = glm::vec3(json["globalScale"][0].get<float>(), json["globalScale"][1].get<float>(), json["globalScale"][2].get<float>());
            newNode->globalPosition = globalPosition;
            newNode->globalRotation = globalRotation;
            newNode->globalScale = globalScale;
            newNode->globalMatrix = newNode->computeMatrixFromTransform(globalPosition, globalRotation, globalScale);
            newNode->commitGlobalUpdate();

            glm::vec3 localPosition = VEC3_ZERO;
            glm::vec3 localRotation = VEC3_ZERO;
            glm::vec3 localScale = VEC3_ONE;
            if (json.contains("localPosition")) localPosition = glm::vec3(json["localPosition"][0].get<float>(), json["localPosition"][1].get<float>(), json["localPosition"][2].get<float>());
            if (json.contains("localRotation")) localRotation = glm::vec3(json["localRotation"][0].get<float>(), json["localRotation"][1].get<float>(), json["localRotation"][2].get<float>());
            if (json.contains("localScale")) localScale = glm::vec3(json["localScale"][0].get<float>(), json["localScale"][1].get<float>(), json["localScale"][2].get<float>());
            newNode->localPosition = localPosition;
            newNode->localRotation = localRotation;
            newNode->localScale = localScale;
            newNode->localMatrix = newNode->computeMatrixFromTransform(localPosition, localRotation, localScale);
            newNode->commitLocalUpdate();

            if (json.contains("color")) newNode->color = glm::vec3(json["color"][0].get<float>(), json["color"][1].get<float>(), json["color"][2].get<float>());
            if (json.contains("radiance")) newNode->radiance = json["radiance"].get<float>();
            if (json.contains("isOn")) newNode->isOn = json["radiance"].get<bool>();

            return newNode;
        }
};

#endif
