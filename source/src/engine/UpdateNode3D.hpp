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

            return newNode;
        }

};

#endif
