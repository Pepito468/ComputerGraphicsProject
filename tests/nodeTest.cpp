#include "Node.hpp"
#include "Light.hpp"
#include <iostream>

int main() {
    std::cout << "NODE TEST" << std::endl << std::endl;

    Node *parent = new Light();
    Node *child = new Node();

    std::cout << "parent UUID: " << parent->UUID << std::endl;
    std::cout << "child UUID: " << child->UUID << std::endl;

    parent->adopt(child);
    parent->adopt(child);
    parent->disown(child);
    parent->disown(child);

    delete parent;
    delete child;

    std::cout << std::endl << "TRANSFORM TEST" << std::endl << std::endl;

    Node3D node3d = Node3D();
    node3d.translate(glm::vec3(2, 2, 0));
    node3d.rotateX(glm::radians(60.0f));
    node3d.rotateY(glm::radians(45.0f));
    node3d.rotateZ(glm::radians(-45.0f));
    node3d.setScale(glm::vec3(1, 2, 3));
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", node3d.matrix[j][i]);
        std::cout << std::endl;
    }

    std::cout << std::endl;
    glm::vec3 pos = node3d.getPosition();
    glm::vec3 rot = node3d.getRotation();
    glm::vec3 sca = node3d.getScale();
    printf("POS: %.2f %.2f %.2f\n", pos.x, pos.y, pos.z);
    printf("ROT: %.2f %.2f %.2f\n", rot.x, rot.y, rot.z);
    printf("SCA: %.2f %.2f %.2f\n\n", sca.x, sca.y, sca.z);
    node3d.setRotation(glm::radians(45.0f), glm::radians(15.0f), glm::radians(90.0f));
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", node3d.matrix[j][i]);
        std::cout << std::endl;
    }
    printf("ROT: %.2f %.2f %.2f\n\n", rot.x, rot.y, rot.z);

    glm::vec3 point = glm::vec3(2, 2, 2);
    glm::vec3 localPos = node3d.toLocalSpace(point);
    printf("LOCAL: %.2f %.2f %.2f\n\n", localPos.x, localPos.y, localPos.z);

    std::cout << std::endl << "TRANSFORM TEST 2" << std::endl << std::endl;

    Node3D a = Node3D(glm::vec3(1, 1, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
    a.rotateZ(std::numbers::pi/4);
    a.setScale(glm::vec3(2, -1, 1));
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", a.matrix[j][i]);
        std::cout << std::endl;
    }

    std::cout << std::endl << "END TEST" << std::endl << std::endl;
    
    return 0;
}
