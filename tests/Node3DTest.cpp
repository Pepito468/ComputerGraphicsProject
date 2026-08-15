#include "Node3D.hpp"
#include "Relations.hpp"
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
                node3d->fatherMatrix = fatherTransformMatrix;
                node3d->updateGlobalMatrixFromLocal();
                node3d->updateGlobalTransformPropertiesFromGlobalMatrix();
                fatherTransformMatrix = node3d->globalMatrix;
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
    node3d.localRotateX(glm::radians(45.0f));
    node3d.localRotateY(glm::radians(15.0f));
    node3d.localRotateZ(glm::radians(90.0f));
    node3d.localScaleAll(glm::vec3(1, 2, 3));

    // Commit transform
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", node3d.globalMatrix[j][i]);
        std::cout << std::endl;
    }
    std::cout << std::endl;

    // Test transform
    glm::vec3 pos = node3d.globalPosition;
    printf("POS: %.2f %.2f %.2f\n", pos.x, pos.y, pos.z);
    assert(glm::all(glm::epsilonEqual(pos, glm::vec3(2, 2, 0), epsilon)));
    glm::vec3 rot = node3d.globalRotation;
    printf("ROT: %.2f %.2f %.2f\n", rot.x, rot.y, rot.z);
    assert(glm::all(glm::epsilonEqual(rot, glm::vec3(0.79, 0.26, 1.57), epsilon)));
    glm::vec3 sca = node3d.globalScale;
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
    a.localRotateZ(std::numbers::pi/4);
    a.globalScaleAll(glm::vec3(2, -1, 1));
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", a.globalMatrix[j][i]);
        std::cout << std::endl;
    }
    printf("\n");

    printf("POS: %.2f %.2f %.2f\n", a.globalPosition.x, a.globalPosition.y, a.globalPosition.z);
    assert(glm::all(glm::epsilonEqual(a.globalPosition, glm::vec3(1, 1, 0), epsilon)));
    printf("ROT: %.2f %.2f %.2f\n", a.globalRotation.x, a.globalRotation.y, a.globalRotation.z);
    assert(glm::all(glm::epsilonEqual(a.globalRotation, glm::vec3(0, 0, -std::numbers::pi/4), epsilon)));
    printf("SCA: %.2f %.2f %.2f\n\n", a.globalScale.x, a.globalScale.y, a.globalScale.z);
    assert(glm::all(glm::epsilonEqual(a.globalScale, glm::vec3(2, 1, 1), epsilon)));


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
    child.localTranslate(glm::vec3(2, 0, 0));
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
            printf("%.2f\t", father.globalMatrix[j][i]);
        printf("\n");
    }
    printf("\n");
    printf("CHILD\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", child.globalMatrix[j][i]);
        printf("\n");
    }
    printf("\n");
    printf("GRANDCHILD\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%.2f\t", grandchild.globalMatrix[j][i]);
        printf("\n");
    }
    printf("\n\n");
    printf("Father: %.4f %.4f %.4f\n", father.globalPosition.x, father.globalPosition.y, father.globalPosition.z);
    printf("Child: %.4f %.4f %.4f\n", child.globalPosition.x, child.globalPosition.y, child.globalPosition.z);
    printf("Grandchild: %.4f %.4f %.4f\n", grandchild.globalPosition.x, grandchild.globalPosition.y, grandchild.globalPosition.z);
    assert(glm::all(glm::epsilonEqual(father.globalPosition, glm::vec3(1, 0, 0), epsilon)));
    assert(glm::all(glm::epsilonEqual(child.globalPosition, glm::vec3(1, 2, 0), epsilon)));
    assert(glm::all(glm::epsilonEqual(grandchild.globalPosition, glm::vec3(1, 2, 1), epsilon)));

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
    father2.globalScaleAll({2, 2, 2});
    child2.localTranslate({1, 1, 0});
    child2.globalRotateZ(glm::radians(45.0f));
    printf("LOCAL:\n");
    printMatrix(child2.localMatrix);
    printf("\n");
    printf("GLOBAL:\n");
    printMatrix(child2.globalMatrix);
    printf("\n");
    printf("GPOS: %.4f %.4f %.4f\n", child2.globalPosition.x, child2.globalPosition.y, child2.globalPosition.z);
    assert(glm::all(glm::epsilonEqual(child2.globalPosition, {1, 3.8284, 0}, epsilon)));
    printf("GROT: %.4f %.4f %.4f\n", child2.globalRotation.x, child2.globalRotation.y, child2.globalRotation.z);
    assert(glm::all(glm::epsilonEqual(child2.globalRotation, {0, 0, glm::radians(90.0f)}, epsilon)));
    printf("GSCA: %.4f %.4f %.4f\n", child2.globalScale.x, child2.globalScale.y, child2.globalScale.z);
    assert(glm::all(glm::epsilonEqual(child2.globalScale, {2, 2, 2}, epsilon)));
    printf("LPOS: %.4f %.4f %.4f\n", child2.localPosition.x, child2.localPosition.y, child2.localPosition.z);
    assert(glm::all(glm::epsilonEqual(child2.localPosition, {1, 1, 0}, epsilon)));
    printf("LROT: %.4f %.4f %.4f\n", child2.localRotation.x, child2.localRotation.y, child2.localRotation.z);
    assert(glm::all(glm::epsilonEqual(child2.localRotation, {0, 0, glm::radians(45.0f)}, epsilon)));
    printf("LSCA: %.4f %.4f %.4f\n", child2.localScale.x, child2.localScale.y, child2.localScale.z);
    assert(glm::all(glm::epsilonEqual(child2.localScale, {1, 1, 1}, epsilon)));


    info("END OF NODE3D TEST");
}
