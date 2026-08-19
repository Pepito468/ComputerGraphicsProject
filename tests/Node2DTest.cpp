#include "Node2D.hpp"
#include <glm/ext/vector_float2.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vector_relational.hpp>

class MockEngine {

    public: 
        /** Recomputes the matrixes after the nodes are moved and the hierarchy has changed */
        void recompute2DNodeHierarchy(Node* node, glm::mat4 fatherTransformMatrix) {

            // Update self matrix
            if (Node2D* node2d = dynamic_cast<Node2D*>(node)) {
                node2d->updateFatherMatrix(fatherTransformMatrix);
                return;
            }

            // Propagate to children
            for (Node *child : node->children) {
                recompute2DNodeHierarchy(child, fatherTransformMatrix);
            }
        }
};

void printMatrix(glm::mat4 m) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%.2f\t", m[j][i]);
            if (j == 1) j++;
        }
        printf("\n");
        if (i == 1) i++;
    }
}

int main() {


    const float epsilon = 0.01f;

    info("STARTING NODE2D TEST");

    // Test 1
    Node2D node = Node2D();
    node.localTranslate(glm::vec2(1, 2));
    node.setGlobalRotation(glm::radians(45.0f));
    node.localTranslate(glm::vec2(1, 2));
    node.setLocalScale(glm::vec2(2, 3));
    printMatrix(node.getGlobalMatrix());


    glm::vec2 pos = node.getGlobalPosition();
    printf("POS: %.2f %.2f\n", pos.x, pos.y);
    assert(glm::all(glm::epsilonEqual(pos, glm::vec2(2, 4), epsilon)));
    float rot = node.getGlobalRotation();
    printf("ROT: %.2f\n", rot);
    assert(glm::epsilonEqual(rot, glm::radians(45.0f), epsilon));
    glm::vec2 sca = node.getGlobalScale();
    printf("SCA: %.2f %.2f\n\n", sca.x, sca.y);
    assert(glm::all(glm::epsilonEqual(sca, glm::vec2(2, 3), epsilon)));

    // Test 2
    MockEngine engine = MockEngine();
    Node2D father2 = Node2D();
    Node2D child2 = Node2D();

    father2.adopt(&child2);
    engine.recompute2DNodeHierarchy(&father2, MAT4_I);
    father2.localTranslate({1, 1});
    father2.localRotate(glm::radians(45.0f));
    father2.globalScaleAll({2, 2});
    child2.setLocalPosition({1, 1});
    child2.globalRotate(glm::radians(45.0f));
    printf("LOCAL:\n");
    printMatrix(child2.getLocalMatrix());
    printf("\n");
    printf("GLOBAL:\n");
    printMatrix(child2.getGlobalMatrix());
    printf("\n");
    printf("GPOS: %.4f %.4f\n", child2.getGlobalPosition().x, child2.getGlobalPosition().y);
    assert(glm::all(glm::epsilonEqual(child2.getGlobalPosition(), {1, 3.8284}, epsilon)));
    printf("GROT: %.4f\n", child2.getGlobalRotation());
    assert(glm::epsilonEqual(child2.getGlobalRotation(), glm::radians(90.0f), epsilon));
    printf("GSCA: %.4f %.4f\n", child2.getGlobalScale().x, child2.getGlobalScale().y);
    assert(glm::all(glm::epsilonEqual(child2.getGlobalScale(), {2, 2}, epsilon)));
    printf("LPOS: %.4f %.4f\n", child2.getLocalPosition().x, child2.getLocalPosition().y);
    assert(glm::all(glm::epsilonEqual(child2.getLocalPosition(), {1, 1}, epsilon)));
    printf("LROT: %.4f\n", child2.getLocalRotation());
    assert(glm::epsilonEqual(child2.getLocalRotation(), glm::radians(45.0f), epsilon));
    printf("LSCA: %.4f %.4f\n", child2.getLocalScale().x, child2.getLocalScale().y);
    assert(glm::all(glm::epsilonEqual(child2.getLocalScale(), {1, 1}, epsilon)));

    info("END OF NODE2D TEST");

    return 0;
}
