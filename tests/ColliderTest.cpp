#include <cmath>
#include <format>
#include "Collider.hpp"
#include "Debug.hpp"
#include "Relations.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>


void boxTest() {
    println("\tBOX COLLIDER TEST");
    BoxCollider box = BoxCollider();

    println("\t\tBOUNDS TEST -- Basic");

    for (float x = -1; x < 1; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1; z < 1; z += 0.1f) {
                const glm::vec3 p = {x, y, z};
                glm::vec3 limits = glm::vec3(0.5f);
                if (epsilonLessThanEqual(glm::abs(p), limits)) {
                    _assert(box.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!box.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }

    println("\t\tBOUNDS TEST -- Rotated box");
    box.rotateY(glm::radians(45.0f));
    const double d = 0.5f * sqrt(2.0f);
    for (float x = -1; x < 1; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1; z < 1; z += 0.1f) {
                const glm::vec3 p = {x, y, z};
                if (epsilonGreaterThanEqual(z, std::abs(x) - d) && epsilonLessThanEqual(z, -std::abs(x) + d) && epsilonLessThanEqual(std::abs(y), 0.5f)) {
                    _assert(box.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!box.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }
    box.rotateY(glm::radians(-45.0f));

    println("\t\tBOUNDS TEST -- Elongated box");
    box.width = 2;
    for (float x = -1.5f; x < 1.5f; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1; z < 1; z += 0.1f) {
                const glm::vec3 p = {x, y, z};
                constexpr glm::vec3 limits = {1, 0.5f, 0.5f};
                if (epsilonLessThanEqual(glm::abs(p), limits)) {
                    _assert(box.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!box.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }

    println("\t\tBOUNDS TEST -- Non-uniformly scaled box");
    box.scaleAll(glm::vec3(1, 1, 2));
    for (float x = -1.5f; x < 1.5f; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1.5f; z < 1.5f; z += 0.1f) {
                const glm::vec3 p = {x, y, z};
                constexpr glm::vec3 limits = {1, 0.5f, 1};
                if (epsilonLessThanEqual(glm::abs(p), limits)) {
                    _assert(box.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!box.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }
    box.scaleAll(glm::vec3(1, 1, 0.5f));
    box.width = 1;

    println("\t\tBOUNDS TEST -- Translated box");
    box.translate(glm::vec3(0.5f));
    for (float x = -0.5f; x < 1.5f; x += 0.1f) {
        for (float y = -0.5f; y < 1.5f; y += 0.1f) {
            for (float z = -0.5f; z < 1.5f; z += 0.1f) {
                const glm::vec3 p = glm::vec3(x, y, z);
                if (epsilonBetween(glm::vec3(0), p, glm::vec3(1))) {
                    _assert(box.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!box.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }
    box.translate(glm::vec3(-0.5f));

    println("\t\tSPHERE BOUNDS TEST -- Basic");
    SphereBounds* sB = box.getSphereBounds();
    glm::vec3 v = glm::vec3(box.width, box.height, box.depth) * box.scale * glm::vec3(0.5f);
    float expectedR = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    _assert(epsilonEqual(sB->center, box.position), std::format("Wrong center: {} vs {}", sB->center, box.position));
    _assert(epsilonEqual(sB->radius, expectedR), std::format("Wrong radius: {} vs {}", sB->radius, expectedR));
    delete sB;

    println("\t\tSPHERE BOUNDS TEST -- Rotated and translated box");
    //Radius shouldn't change
    box.rotateY(glm::radians(45.0f));
    box.rotateX(glm::radians(-64.0f));
    box.translate(glm::vec3(18, 46, -0.57f));
    sB = box.getSphereBounds();

    _assert(epsilonEqual(sB->center, box.position), std::format("Wrong center: {} vs {}", sB->center, box.position));
    _assert(epsilonEqual(sB->radius, expectedR), std::format("Wrong radius: {} vs {}", sB->radius, expectedR));
    delete sB;
    box.translate(-glm::vec3(18, 46, -0.57f));
    box.rotateX(glm::radians(64.0f));
    box.rotateY(glm::radians(-45.0f));

    println("\t\tSPHERE BOUNDS TEST -- Elongated box");
    box.width = 1.7f;
    box.depth = 0.45f;
    box.scaleAll(glm::vec3(16.3f, 0.42f, 61.54f));
    sB = box.getSphereBounds();
    v = glm::vec3(box.width, box.height, box.depth) * box.scale * glm::vec3(0.5f);
    expectedR = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    _assert(epsilonEqual(sB->center, box.position), std::format("Wrong center: {} vs {}", sB->center, box.position));
    _assert(epsilonEqual(sB->radius, expectedR), std::format("Wrong radius: {} vs {}", sB->radius, expectedR));
    delete sB;
    box.width = 1;
    box.depth = 1;
    box.scaleAll(glm::vec3(1/16.3f, 1/0.42f, 1/61.54f));

    println("\t\tAABB TEST -- Basic");
    AABBExtents* aabb = box.getAABBExtents();
    _assert(epsilonEqual(aabb->xMax, 0.5f), std::format("Wrong xMax: {} vs {}", aabb->xMax, 0.5f));
    _assert(epsilonEqual(aabb->yMax, 0.5f), std::format("Wrong yMax: {} vs {}", aabb->yMax, 0.5f));
    _assert(epsilonEqual(aabb->zMax, 0.5f), std::format("Wrong zMax: {} vs {}", aabb->zMax, 0.5f));
    _assert(epsilonEqual(aabb->xMin, -0.5f), std::format("Wrong xMin: {} vs {}", aabb->xMin, -0.5f));
    _assert(epsilonEqual(aabb->yMin, -0.5f), std::format("Wrong yMin: {} vs {}", aabb->yMin, -0.5f));
    _assert(epsilonEqual(aabb->zMin, -0.5f), std::format("Wrong zMin: {} vs {}", aabb->zMin, -0.5f));
    delete aabb;

    println("\t\tAABB TEST -- Translated box");
    box.translate(glm::vec3(1, 2, -3));
    aabb = box.getAABBExtents();
    _assert(epsilonEqual(aabb->xMax, 1.5f), std::format("Wrong xMax: {} vs {}", aabb->xMax, 1.5f));
    _assert(epsilonEqual(aabb->yMax, 2.5f), std::format("Wrong yMax: {} vs {}", aabb->yMax, 2.5f));
    _assert(epsilonEqual(aabb->zMax, -2.5f), std::format("Wrong zMax: {} vs {}", aabb->zMax, -2.5f));
    _assert(epsilonEqual(aabb->xMin, 0.5f), std::format("Wrong xMin: {} vs {}", aabb->xMin, 0.5f));
    _assert(epsilonEqual(aabb->yMin, 1.5f), std::format("Wrong yMin: {} vs {}", aabb->yMin, 1.5f));
    _assert(epsilonEqual(aabb->zMin, -3.5f), std::format("Wrong zMin: {} vs {}", aabb->zMin, -3.5f));
    delete aabb;
    box.translate(-glm::vec3(1, 2, -3));

    println("\t\tAABB TEST -- Rotated box");
    box.rotateY(glm::radians(45.0f));
    aabb = box.getAABBExtents();
    _assert(epsilonEqual(aabb->xMax, 0.5f * (float)sqrt(2)), std::format("Wrong xMax: {} vs {}", aabb->xMax, 0.5f * (float)sqrt(2)));
    _assert(epsilonEqual(aabb->yMax, 0.5f), std::format("Wrong yMax: {} vs {}", aabb->yMax, 0.5f));
    _assert(epsilonEqual(aabb->zMax, 0.5f * (float)sqrt(2)), std::format("Wrong zMax: {} vs {}", aabb->zMax, 0.5f));
    _assert(epsilonEqual(aabb->xMin, -0.5f * (float)sqrt(2)), std::format("Wrong xMin: {} vs {}", aabb->xMin, -0.5f * (float)sqrt(2)));
    _assert(epsilonEqual(aabb->yMin, -0.5f), std::format("Wrong yMin: {} vs {}", aabb->yMin, -0.5f));
    _assert(epsilonEqual(aabb->zMin, -0.5f * (float)sqrt(2)), std::format("Wrong zMin: {} vs {}", aabb->zMin, -0.5f * (float)sqrt(2)));
    delete aabb;
    box.rotateY(glm::radians(-45.0f));

    println("\t\tAABB TEST -- Scaled box");
    box.scaleAll(glm::vec3(1.0f, 2.0f, 3.0f));
    box.width = 1.5f;
    aabb = box.getAABBExtents();
    _assert(epsilonEqual(aabb->xMax, 0.5f * 1.5f), std::format("Wrong xMax: {} vs {}", aabb->xMax, 0.5f * 1.5f));
    _assert(epsilonEqual(aabb->yMax, 0.5f * 2), std::format("Wrong yMax: {} vs {}", aabb->yMax, 0.5f * 2));
    _assert(epsilonEqual(aabb->zMax, 0.5f * 3), std::format("Wrong zMax: {} vs {}", aabb->zMax, 0.5f * 3));
    _assert(epsilonEqual(aabb->xMin, -0.5f * 1.5f), std::format("Wrong xMin: {} vs {}", aabb->xMin, -0.5f * 1.5f));
    _assert(epsilonEqual(aabb->yMin, -0.5f * 2), std::format("Wrong yMin: {} vs {}", aabb->yMin, -0.5f * 2));
    _assert(epsilonEqual(aabb->zMin, -0.5f * 3), std::format("Wrong zMin: {} vs {}", aabb->zMin, -0.5f * 3));
    delete aabb;
    box.scaleAll(glm::vec3(1.0f, 1/2.0f, 1/3.0f));
    box.width = 1.0f;

    println("\t\tPOINTS TEST -- Basic");
    PointSet* points = box.getPointSet();
    _assert(points->size() >= POINT_COUNT, std::format("Too few points: {} vs {}", points->size(), POINT_COUNT));
    for ( glm::vec3 point : *points)
    {
        _assert(box.inBounds(point), std::format("{} Not in bounds", point));
    }
    delete points;

    println("\t\tPOINTS TEST -- Funky");
    box.translate(glm::vec3(1.0f, 2.0f, 3.0f));
    box.rotateY(glm::radians(45.0f));
    box.rotateX(glm::radians(-45.0f));
    box.rotateZ(glm::radians(65.0f));
    box.scaleAll(glm::vec3(1.0f, 2.0f, 3.0f));
    points = box.getPointSet();
    _assert(points->size() >= POINT_COUNT, std::format("Too few points: {} vs {}", points->size(), POINT_COUNT));

    for (glm::vec3 point : *points)
    {
        _assert(box.inBounds(point), std::format("{} Not in bounds", point));
    }
    delete points;

    println("\tEND BOX COLLIDER TEST");
}

int main() {
    println("COLLIDER TEST");
    boxTest();
    println("END COLLIDER TEST");
}

