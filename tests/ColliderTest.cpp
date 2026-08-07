#include <cmath>
#include <format>
#include "Debug.hpp"
#include "Collider.hpp"
#include <glm/glm.hpp>

void boxTest() {
    println("BOX COLLIDER TEST");
    BoxCollider box;

    // Bounds Test
    println("BOUNDS TEST -- Basic");
    for (float x = -1; x < 1; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1; z < 1; z += 0.1f) {
                if (std::abs(x) <= 0.5f && std::abs(y) <= 0.5f && std::abs(z) <= 0.5f) {
                    _assert(box.inBounds(glm::vec3(x, y, z)), std::format("[{}, {}, {}] Not in bounds", x, y, z));
                } else {
                    _assert(!box.inBounds(glm::vec3(x, y, z)), std::format("[{}, {}, {}] In bounds", x, y, z));
                }
            }
        }
    }

    println("BOUNDS TEST -- Rotated box");
    box.rotateY(glm::radians(45.0f));
    const double d = 0.5f * sqrt(2.0f);
    for (float x = -1; x < 1; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1; z < 1; z += 0.1f) {
                if (y >= std::abs(x) - d && y <= -std::abs(x) + d && std::abs(z) <= 0.5f) {
                    assert(box.inBounds(glm::vec3(x, y, z)));
                } else {
                    assert(!box.inBounds(glm::vec3(x, y, z)));
                }
            }
        }
    }

    println("BOUNDS TEST -- Scaled box");
    box.rotateY(glm::radians(-45.0f));
    box.width = 2;
    for (float x = -1.5f; x < 1.5f; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1; z < 1; z += 0.1f) {
                if (std::abs(x) <= 1 && std::abs(y) <= 0.5f && std::abs(z) <= 0.5f) {
                    assert(box.inBounds(glm::vec3(x, y, z)));
                } else {
                    assert(!box.inBounds(glm::vec3(x, y, z)));
                }
            }
        }
    }

    println("BOUNDS TEST -- Non-uniformely scaled box");
    box.scaleAll(glm::vec3(1, 1, 2));
    for (float x = -1.5f; x < 1.5f; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1.5f; z < 1.5f; z += 0.1f) {
                if (std::abs(x) <= 1 && std::abs(y) <= 0.5f && std::abs(z) <= 1) {
                    assert(box.inBounds(glm::vec3(x, y, z)));
                } else {
                    assert(!box.inBounds(glm::vec3(x, y, z)));
                }
            }
        }
    }

    println("BOUNDS TEST -- Translated non-uniformely scaled box");
    box.scaleAll(glm::vec3(1, 1, 0.5f));
    box.width = 1;
    box.translate(glm::vec3(0.5f));
    for (float x = -0.5f; x < 1.5f; x += 0.1f) {
        for (float y = -0.5f; y < 1.5f; y += 0.1f) {
            for (float z = -0.5f; z < 1.5f; z += 0.1f) {
                if (0 <= x && x <= 1 && 0 <= y && y <= 1 && 0 <= z && z <= 1) {
                    assert(box.inBounds(glm::vec3(x, y, z)));
                } else {
                    assert(!box.inBounds(glm::vec3(x, y, z)));
                }
            }
        }
    }

    constexpr float epsilon = 0.0001f;
    //Sphere bounds test
    SphereBounds* sB = box.getSphereBounds();
    //assert(glm::epsilonEqual());
}

int main() {
    println("COLLIDER TEST");
    boxTest();
    println("END COLLIDER TEST");
}

