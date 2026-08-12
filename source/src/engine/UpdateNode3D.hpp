#ifndef ENGINE_UPDATENODE3D_H
#define ENGINE_UPDATENODE3D_H

#include "Node3D.hpp"
#include <string>

class UpdateNode3D: public Node3D {

    public:

        /** This method will be called every frame by the Engine */
        void (*update)() = nullptr;


        static UpdateNode3D* fromJSON(const nlohmann::json& json) {
            UpdateNode3D *newNode = new UpdateNode3D();

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

            return newNode;
        }

};

#endif
