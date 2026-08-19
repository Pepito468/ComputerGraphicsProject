#include "Node3D.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <ostream>

class MockEngine {

    public: 
        /** Recomputes the matrixes after the nodes are moved and the hierarchy has changed */
        void recompute3DNodeHierarchy(Node* node, glm::mat4 fatherTransformMatrix) {

            // Update self matrix
            if (Node3D* node3d = dynamic_cast<Node3D*>(node)) {
                node3d->updateFatherMatrix(fatherTransformMatrix);
                return;
            }

            // Propagate to children
            for (Node *child : node->children) {
                recompute3DNodeHierarchy(child, fatherTransformMatrix);
            }
        }
};

void printMatrix(glm::mat4 m) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", m[j][i]);
        std::cout << std::endl;
    }
    printf("\n");
}

int main() {

    info("STARTING NODE3D TEST");

    // Set approx
    const float epsilon = 0.01f;

    // Test 1: transform
    Node3D node3d = Node3D();

    // Apply transform
    node3d.localTranslate(glm::vec3(2, 2, 0));
    node3d.setLocalRotation({glm::radians(45.0f), 0, 0});
    node3d.localRotateY(glm::radians(15.0f));
    node3d.localRotateZ(glm::radians(90.0f));
    node3d.localScaleAll(glm::vec3(1, 2, 3));

    // Commit transform
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", node3d.getGlobalMatrix()[j][i]);
        std::cout << std::endl;
    }
    std::cout << std::endl;

    // Test transform
    glm::vec3 pos = node3d.getGlobalPosition();
    printf("POS: %.2f %.2f %.2f\n", pos.x, pos.y, pos.z);
    assert(glm::all(glm::epsilonEqual(pos, glm::vec3(2, 2, 0), epsilon)));
    glm::vec3 rot = node3d.getGlobalRotation();
    printf("ROT: %.2f %.2f %.2f\n", rot.x, rot.y, rot.z);
    assert(glm::all(glm::epsilonEqual(rot, glm::vec3(0.79, 0.26, 1.57), epsilon)));
    glm::vec3 sca = node3d.getGlobalScale();
    printf("SCA: %.2f %.2f %.2f\n\n", sca.x, sca.y, sca.z);
    assert(glm::all(glm::epsilonEqual(sca, glm::vec3(1, 2, 3), epsilon)));

    // Compute local point
    glm::vec3 point = glm::vec3(2, 2, 2);
    glm::vec3 localPos = node3d.toLocalSpace(point);
    printf("LOCAL: %.2f %.2f %.2f\n\n", localPos.x, localPos.y, localPos.z);

    // Test local point
    assert(glm::all(glm::epsilonEqual(localPos, glm::vec3(-0.37, 0.71, 0.46), epsilon)));

    // Test2: transform 2
    Node3D a = Node3D(glm::vec3(1, 1, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
    a.localRotateZ(std::numbers::pi/4);
    a.globalScaleAll(glm::vec3(2, -1, 1));
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", a.getGlobalMatrix()[j][i]);
        std::cout << std::endl;
    }
    printf("\n");

    printf("POS: %.2f %.2f %.2f\n", a.getGlobalPosition().x, a.getGlobalPosition().y, a.getGlobalPosition().z);
    assert(glm::all(glm::epsilonEqual(a.getGlobalPosition(), glm::vec3(1, 1, 0), epsilon)));
    printf("ROT: %.2f %.2f %.2f\n", a.getGlobalRotation().x, a.getGlobalRotation().y, a.getGlobalRotation().z);
    assert(glm::all(glm::epsilonEqual(a.getGlobalRotation(), glm::vec3(0, 0, -3*std::numbers::pi/4), epsilon)));
    printf("SCA: %.2f %.2f %.2f\n\n", a.getGlobalScale().x, a.getGlobalScale().y, a.getGlobalScale().z);
    assert(glm::all(glm::epsilonEqual(a.getGlobalScale(), glm::vec3(2, 1, 1), epsilon)));


    // Test 3: children
    MockEngine engine = MockEngine();
    Node3D father = Node3D();
    father.name = "father";
    Node3D child = Node3D();
    child.name = "child";
    Node between = Node();
    Node3D grandchild = Node3D();
    grandchild.name = "grandchild";
    father.adopt(&child);
    child.adopt(&between);
    // between will be adopted later

    // The transform itself is very easy to visualize even mentally
    father.globalTranslate(glm::vec3(1, 0, 0));
    child.setLocalPosition(glm::vec3(2, 0, 0));
    grandchild.localTranslate(glm::vec3(1, 0, 0));
    father.localRotateY(glm::radians(90.0f));
    child.localRotateZ(glm::radians(90.0f));
    father.localRotateZ(glm::radians(90.0f));

    // !!! Late adoption !!!
    between.adopt(&grandchild);
    engine.recompute3DNodeHierarchy(&father, MAT4_I);

    printf("FATHER\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", father.getGlobalMatrix()[j][i]);
        printf("\n");
    }
    printf("\n");
    printf("CHILD\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", child.getGlobalMatrix()[j][i]);
        printf("\n");
    }
    printf("\n");
    printf("GRANDCHILD\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", grandchild.getGlobalMatrix()[j][i]);
        printf("\n");
    }
    printf("\n\n");
    printf("Father: %.4f %.4f %.4f\n", father.getGlobalPosition().x, father.getGlobalPosition().y, father.getGlobalPosition().z);
    printf("Child: %.4f %.4f %.4f\n", child.getGlobalPosition().x, child.getGlobalPosition().y, child.getGlobalPosition().z);
    printf("Grandchild: %.4f %.4f %.4f\n", grandchild.getGlobalPosition().x, grandchild.getGlobalPosition().y, grandchild.getGlobalPosition().z);
    assert(glm::all(glm::epsilonEqual(father.getGlobalPosition(), glm::vec3(1, 0, 0), epsilon)));
    assert(glm::all(glm::epsilonEqual(child.getGlobalPosition(), glm::vec3(1, 0, -2), epsilon)));
    assert(glm::all(glm::epsilonEqual(grandchild.getGlobalPosition(), glm::vec3(0, 0, -2), epsilon)));

    // Test 4: Collider

    Node3D box = Node3D();
    Node3D box2 = Node3D();

    box.localScaleAll({16.3f, 0.42f, 61.54f});
    box.localTranslate({-18, -46, 0.57f});
    box2.localRotateX(glm::radians(64.0f));
    box2.localRotateY(glm::radians(-45.0f));
    box2.localScaleAll({16.3f, 0.42f, 61.54f});
    // Should have the same scale
    assert(glm::all(glm::epsilonEqual(box.globalScale, box2.globalScale, epsilon)));

    // Test 5: global and local
    Node3D father2 = Node3D();
    Node3D child2 = Node3D();

    father2.adopt(&child2);
    engine.recompute3DNodeHierarchy(&father2, MAT4_I);
    father2.localTranslate({1, 1, 0});
    father2.localRotateZ(glm::radians(45.0f));
    father2.setGlobalScale({2, 2, 2});
    child2.localTranslate({1, 1, 0});
    child2.globalRotateZ(glm::radians(45.0f));
    printf("LOCAL:\n");
    printMatrix(child2.getLocalMatrix());
    printf("\n");
    printf("GLOBAL:\n");
    printMatrix(child2.getGlobalMatrix());
    printf("\n");
    printf("GPOS: %.4f %.4f %.4f\n", child2.getGlobalPosition().x, child2.getGlobalPosition().y, child2.getGlobalPosition().z);
    assert(glm::all(glm::epsilonEqual(child2.getGlobalPosition(), {1, 3.8284, 0}, epsilon)));
    printf("GROT: %.4f %.4f %.4f\n", child2.getGlobalRotation().x, child2.getGlobalRotation().y, child2.getGlobalRotation().z);
    assert(glm::all(glm::epsilonEqual(child2.getGlobalRotation(), {0, 0, glm::radians(90.0f)}, epsilon)));
    printf("GSCA: %.4f %.4f %.4f\n", child2.getGlobalScale().x, child2.getGlobalScale().y, child2.getGlobalScale().z);
    assert(glm::all(glm::epsilonEqual(child2.getGlobalScale(), {2, 2, 2}, epsilon)));
    printf("LPOS: %.4f %.4f %.4f\n", child2.getLocalPosition().x, child2.getLocalPosition().y, child2.getLocalPosition().z);
    assert(glm::all(glm::epsilonEqual(child2.getLocalPosition(), {1, 1, 0}, epsilon)));
    printf("LROT: %.4f %.4f %.4f\n", child2.getLocalRotation().x, child2.getLocalRotation().y, child2.getLocalRotation().z);
    assert(glm::all(glm::epsilonEqual(child2.getLocalRotation(), {0, 0, glm::radians(45.0f)}, epsilon)));
    printf("LSCA: %.4f %.4f %.4f\n", child2.getLocalScale().x, child2.getLocalScale().y, child2.getLocalScale().z);
    assert(glm::all(glm::epsilonEqual(child2.getLocalScale(), {1, 1, 1}, epsilon)));

    // Test 6: gimbal lock
    Node3D gimbal = Node3D();
    // Should throw a gimbal lock warning
    printf("GIMBAL LOCK SHOULD BE THROWN NOW:\n");
    gimbal.localRotateX(glm::radians(90.0f));
    printf("GIMBAL LOCK SHOULD HAVE BEEN THROWN\n");

    info("END OF NODE3D TEST");
}
