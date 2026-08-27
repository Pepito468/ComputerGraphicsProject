#ifndef ENGINE_UPDATENODE3D_H
#define ENGINE_UPDATENODE3D_H

#include "Node3D.hpp"
#include <string>

class UpdateNode3D: public Node3D
{
public:
    /// If the node is to be updated every frame
    bool isActive = true;

    /// This method will be called every frame by the Engine if the node is active
    virtual void update() {
        warning(std::format("Update Method for UpdateNode3D [{}] has not been changed from default", this->UUID), true);
    }

    static UpdateNode3D* fromJSON(const nlohmann::json& json, UpdateNode3D* node = nullptr) {
        UpdateNode3D *newNode = node ? node: new UpdateNode3D();

        Node3D::fromJSON(json, newNode);

        return newNode;
    }
};

#endif
