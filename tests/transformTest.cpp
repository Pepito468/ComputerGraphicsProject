#include "Node3D.hpp"
#include "Engine.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <ostream>

int main() {
    std::cout << std::endl << "TRANSFORM TEST" << std::endl << std::endl;

    Engine engine = Engine();

    // Set approx
    const float epsilon = 0.01f;

    // Test 1: transform
    Node3D node3d = Node3D();

    // Apply transform
    node3d.translate(glm::vec3(2, 2, 0));
    node3d.rotateX(glm::radians(45.0f));
    node3d.rotateY(glm::radians(15.0f));
    node3d.rotateZ(glm::radians(90.0f));
    node3d.scaleAll(glm::vec3(1, 2, 3));

    // Commit transform
    engine.updateWorldTransform(&node3d, MAT4_I);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", node3d.matrix[j][i]);
        std::cout << std::endl;
    }
    std::cout << std::endl;

    // Test transform
    glm::vec3 pos = node3d.position;
    printf("POS: %.2f %.2f %.2f\n", pos.x, pos.y, pos.z);
    assert(glm::all(glm::epsilonEqual(pos, glm::vec3(2, 2, 0), epsilon)));
    glm::vec3 rot = node3d.rotation;
    printf("ROT: %.2f %.2f %.2f\n", rot.x, rot.y, rot.z);
    assert(glm::all(glm::epsilonEqual(rot, glm::vec3(0.79, 0.26, 1.57), epsilon)));
    glm::vec3 sca = node3d.scale;
    printf("SCA: %.2f %.2f %.2f\n\n", sca.x, sca.y, sca.z);
    assert(glm::all(glm::epsilonEqual(sca, glm::vec3(1, 2, 3), epsilon)));


    // Compute local point
    glm::vec3 point = glm::vec3(2, 2, 2);
    glm::vec3 localPos = node3d.toLocalSpace(point);
    printf("LOCAL: %.2f %.2f %.2f\n\n", localPos.x, localPos.y, localPos.z);

    // Test local point
    assert(glm::all(glm::epsilonEqual(localPos, glm::vec3(1.41, 0.18, 0.46), epsilon)));

    // Test2: transform 2
    Node3D a = Node3D(glm::vec3(1, 1, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
    a.rotateZ(std::numbers::pi/4);
    a.scaleAll(glm::vec3(2, -1, 1));
    engine.updateWorldTransform(&a, MAT4_I);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", a.matrix[j][i]);
        std::cout << std::endl;
    }
    printf("\n");

    printf("POS: %.2f %.2f %.2f\n", a.position.x, a.position.y, a.position.z);
    assert(glm::all(glm::epsilonEqual(a.position, glm::vec3(1, 1, 0), epsilon)));
    printf("ROT: %.2f %.2f %.2f\n", a.rotation.x, a.rotation.y, a.rotation.z);
    assert(glm::all(glm::epsilonEqual(a.rotation, glm::vec3(0, 0, -std::numbers::pi/4), epsilon)));
    printf("SCA: %.2f %.2f %.2f\n\n", a.scale.x, a.scale.y, a.scale.z);
    assert(glm::all(glm::epsilonEqual(a.scale, glm::vec3(2, 1, 1), epsilon)));


    // Test 3: children
    Node3D father = Node3D();
    Node3D child = Node3D();
    Node between = Node();
    Node3D grandchild = Node3D();
    father.adopt(&child);
    child.adopt(&between);
    between.adopt(&grandchild);

    // The transform itself is very easy to visualize even mentally
    father.translate(glm::vec3(1, 0, 0));
    child.translate(glm::vec3(2, 0, 0));
    grandchild.translate(glm::vec3(1, 0, 0));
    father.rotateY(glm::radians(90.0f));
    child.rotateZ(glm::radians(90.0f));
    father.rotateZ(glm::radians(90.0f));

    // also updates children
    engine.updateWorldTransform(&father, MAT4_I);

    printf("FATHER\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", father.matrix[j][i]);
        printf("\n");
    }
    printf("\n");
    printf("CHILD\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", child.matrix[j][i]);
        printf("\n");
    }
    printf("\n");
    printf("GRANDCHILD\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", grandchild.matrix[j][i]);
        printf("\n");
    }
    printf("\n\n");
    assert(glm::all(glm::epsilonEqual(father.position, glm::vec3(1, 0, 0), epsilon)));
    assert(glm::all(glm::epsilonEqual(child.position, glm::vec3(1, 2, 0), epsilon)));
    assert(glm::all(glm::epsilonEqual(grandchild.position, glm::vec3(1, 2, 1), epsilon)));


    std::cout << "END TRANSFORM TEST" << std::endl;
}
