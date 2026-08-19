#ifndef ENGINE_MODEL3D_H
#define ENGINE_MODEL3D_H
#include "Node3D.hpp"

/// A node represented with a 3D model
class Model3D : public Node3D {

    public:

        static Model3D* fromJSON(const nlohmann::json& json) {
            Model3D *newNode = new Model3D();

            if (json.contains("name")) newNode->name = json["name"].get<std::string>();

            glm::vec3 globalPosition = VEC3_ZERO;
            glm::vec3 globalRotation = VEC3_ZERO;
            glm::vec3 globalScale = VEC3_ONE;
            if (json.contains("globalPosition")) globalPosition = glm::vec3(json["globalPosition"][0].get<float>(), json["globalPosition"][1].get<float>(), json["globalPosition"][2].get<float>());
            if (json.contains("globalRotation")) globalRotation = glm::vec3(json["globalRotation"][0].get<float>(), json["globalRotation"][1].get<float>(), json["globalRotation"][2].get<float>());
            if (json.contains("globalScale")) globalScale = glm::vec3(json["globalScale"][0].get<float>(), json["globalScale"][1].get<float>(), json["globalScale"][2].get<float>());
            newNode->setGlobalPosition(globalPosition);
            newNode->setGlobalRotation(globalRotation);
            newNode->setGlobalScale(globalScale);

            return newNode;
        }

        //TODO we need to understand how meshes are loaded
};
#endif
