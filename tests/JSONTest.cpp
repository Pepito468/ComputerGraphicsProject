#include <Libraries.hpp>
#include <cassert>
#include <string>
#include <JsonToNode.hpp>
#include "Debug.hpp"

int main() {

    std::string json = R"(
    {
        "type": "Node3D",
        "name": "Joseph",
        "position": [56.3, 21, 21e3],
        "rotation": [1, 2, 4],
        "scale": [56, 71, 19],
        "children": [
            {
                "type": "Model3D",
                "name": "Holly",
                "children": [
                    {
                        "type": "Node2D",
                        "name": "Jotaro",
                        "globalRotation": 56.0,
                        "children": [
                            {
                                "type": "PerspectiveCamera",
                                "name": "Jolyne",
                                "nearValue": 13.0
                            }
                        ]
                    }
                ]
            },
            {
                "type": "PointLight",
                "name": "Josuke",
                "radius": 3.0
            }
        ]
    }
    )";

    _assert(getNodeFromJSON(json).has_value(), "json parsing failed");
    Node *root = getNodeFromJSON(json).value();
    printf("%s\n", root->name.c_str());
    assert(root->name == "Joseph");
    Node3D *root3d = dynamic_cast<Node3D*>(root);
    printf("%f %f %f\n", root3d->globalPosition.x, root3d->globalPosition.y, root3d->globalPosition.z);
    printf("%f %f %f\n", root3d->globalRotation.x, root3d->globalRotation.y, root3d->globalRotation.z);
    printf("%f %f %f\n", root3d->globalScale.x, root3d->globalScale.y, root3d->globalScale.z);
    for (Node* child : root->children) {
        if (child->name == "Holly") {
            printf("%s\n", child->name.c_str());
            for (Node* child : child->children) {
                printf("%s\n", child->name.c_str());
                assert(child->name == "Jotaro");
                printf("%f\n", dynamic_cast<Node2D*>(child)->globalRotation);
                for (Node* child : child->children) {
                    printf("%s\n", child->name.c_str());
                    assert(child->name == "Jolyne");
                    printf("%f\n", dynamic_cast<PerspectiveCamera*>(child)->nearValue);
                }
            }
        } else {
            printf("%s\n", child->name.c_str());
            assert(child->name == "Josuke");
            printf("%f\n", dynamic_cast<PointLight*>(child)->radius);
        }
    }


    return 0;
}
