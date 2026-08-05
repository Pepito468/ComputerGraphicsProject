#include "Engine.hpp"
#include "Node2D.hpp"
#include <glm/ext/vector_float2.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vector_relational.hpp>

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


    Engine engine = Engine();
    const float epsilon = 0.01f;

    printf("NODE2D TEST\n\n");

    Node2D node = Node2D();
    node.translate(glm::vec2(1, 2));
    node.rotate(glm::radians(45.0f));
    node.translate(glm::vec2(1, 2));
    node.scaleAll(glm::vec2(2, 3));
    engine.update2DWorldTransform(&node, MAT4_I);
    printMatrix(node.matrix);


    glm::vec2 pos = node.position;
    printf("POS: %.2f %.2f\n", pos.x, pos.y);
    assert(glm::all(glm::epsilonEqual(pos, glm::vec2(2, 4), epsilon)));
    float rot = node.rotation;
    printf("ROT: %.2f\n", rot);
    assert(glm::epsilonEqual(rot, glm::radians(45.0f), epsilon));
    glm::vec2 sca = node.scale;
    printf("SCA: %.2f %.2f\n\n", sca.x, sca.y);
    assert(glm::all(glm::epsilonEqual(sca, glm::vec2(2, 3), epsilon)));

    return 0;
}
