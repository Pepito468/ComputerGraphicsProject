#ifndef ENGINE_JSONTOSCENE_H
#define ENGINE_JSONTOSCENE_H

#include <json.hpp>
#include <optional>
#include "Libraries.hpp"

inline std::optional<Node*> getNodeFromJSONObj(const nlohmann::json json) {
    std::string type = json.value("type", "INVALID");

    Node *newNode;

    if (type == "Node") {
        newNode = Node::fromJSON(json);
    } else if (type == "Node3D") {
        newNode = Node3D::fromJSON(json);
    } else if (type == "Node2D") {
        newNode = Node2D::fromJSON(json);
    } else if (type == "PerspectiveCamera") {
        newNode = PerspectiveCamera::fromJSON(json);
    } else if (type == "OrthoCamera") {
        newNode = OrthoCamera::fromJSON(json);
    } else if (type == "Collider") {
        // newNode = Collider::fromJSON(json); TODO
    } else if (type == "Model3D") {
        newNode = Model3D::fromJSON(json);
    } else if (type == "Sprite2D") {
        newNode = Sprite2D::fromJSON(json);
    } else if (type == "Text2D") {
        newNode = Text2D::fromJSON(json);
    } else if (type == "UpdateNode3D") {
        newNode = UpdateNode3D::fromJSON(json);
    } else if (type == "PointLight") {
        newNode = PointLight::fromJSON(json);
    } else if (type == "SpotLight") {
        newNode = SpotLight::fromJSON(json);
    } else if (type == "DirectionalLight") {
        newNode = DirectionalLight::fromJSON(json);
    } else {
        warning("JSON is malformed");
        return std::nullopt;
    }

    if (json.contains("children")) {
        for (const auto& childJSON : json.at("children")) {
            std::optional<Node*> child = getNodeFromJSONObj(childJSON);
            if (!child.has_value()) {
                continue;
            }
            newNode->adopt(child.value());
        }
    }

    return newNode;

}

inline std::optional<Node*> getNodeFromJSON(const std::string& json) {
    try {
        nlohmann::json j = nlohmann::json::parse(json);
        return getNodeFromJSONObj(j);

    } catch (const nlohmann::json::parse_error& e) {
        error(e.what());
        return std::nullopt;
    }

}

#endif
