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
    box.localRotateY(glm::radians(45.0f));
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
    box.localRotateY(glm::radians(-45.0f));

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
    box.localScaleAll(glm::vec3(1, 1, 2));
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
    box.localScaleAll(glm::vec3(1, 1, 0.5f));
    box.width = 1;

    println("\t\tBOUNDS TEST -- Translated box");
    box.localTranslate(glm::vec3(0.5f));
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
    box.localTranslate(glm::vec3(-0.5f));

    println("\t\tSPHERE BOUNDS TEST -- Basic");
    SphereBounds sB = box.getSphereBounds();
    glm::vec3 v = glm::vec3(box.width, box.height, box.depth) * box.getGlobalScale()* glm::vec3(0.5f);
    float expectedR = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    _assert(epsilonEqual(sB.center, box.getGlobalPosition()), std::format("Wrong center: {} vs {}", sB.center, box.getGlobalPosition()));
    _assert(epsilonEqual(sB.radius, expectedR), std::format("Wrong radius: {} vs {}", sB.radius, expectedR));

    println("\t\tSPHERE BOUNDS TEST -- Rotated and translated box");
    //Radius shouldn't change
    box.localRotateY(glm::radians(45.0f));
    box.localRotateX(glm::radians(-64.0f));
    box.localTranslate(glm::vec3(18, 46, -0.57f));
    sB = box.getSphereBounds();

    _assert(epsilonEqual(sB.center, box.getGlobalPosition()), std::format("Wrong center: {} vs {}", sB.center, box.getGlobalPosition()));
    _assert(epsilonEqual(sB.radius, expectedR), std::format("Wrong radius: {} vs {}", sB.radius, expectedR));
    box.localTranslate(-glm::vec3(18, 46, -0.57f));
    box.localRotateX(glm::radians(64.0f));
    box.localRotateY(glm::radians(-45.0f));

    println("\t\tSPHERE BOUNDS TEST -- Elongated box");
    box.width = 1.7f;
    box.depth = 0.45f;
    box.localScaleAll(glm::vec3(16.3f, 0.42f, 61.54f));
    sB = box.getSphereBounds();
    v = glm::vec3(box.width, box.height, box.depth) * box.getGlobalScale ()* glm::vec3(0.5f);
    expectedR = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    _assert(epsilonEqual(sB.center, box.getGlobalPosition()), std::format("Wrong center: {} vs {}", sB.center, box.getGlobalPosition()));
    _assert(epsilonEqual(sB.radius, expectedR), std::format("Wrong radius: {} vs {}", sB.radius, expectedR));
    box.width = 1;
    box.depth = 1;
    box.localScaleAll(glm::vec3(1/16.3f, 1/0.42f, 1/61.54f));

    println("\t\tAABB TEST -- Basic");
    AABBExtents aabb = box.getAABBExtents();
    _assert(epsilonEqual(aabb.xMax, 0.5f), std::format("Wrong xMax: {} vs {}", aabb.xMax, 0.5f));
    _assert(epsilonEqual(aabb.yMax, 0.5f), std::format("Wrong yMax: {} vs {}", aabb.yMax, 0.5f));
    _assert(epsilonEqual(aabb.zMax, 0.5f), std::format("Wrong zMax: {} vs {}", aabb.zMax, 0.5f));
    _assert(epsilonEqual(aabb.xMin, -0.5f), std::format("Wrong xMin: {} vs {}", aabb.xMin, -0.5f));
    _assert(epsilonEqual(aabb.yMin, -0.5f), std::format("Wrong yMin: {} vs {}", aabb.yMin, -0.5f));
    _assert(epsilonEqual(aabb.zMin, -0.5f), std::format("Wrong zMin: {} vs {}", aabb.zMin, -0.5f));

    println("\t\tAABB TEST -- Translated box");
    box.localTranslate(glm::vec3(1, 2, -3));
    aabb = box.getAABBExtents();
    _assert(epsilonEqual(aabb.xMax, 1.5f), std::format("Wrong xMax: {} vs {}", aabb.xMax, 1.5f));
    _assert(epsilonEqual(aabb.yMax, 2.5f), std::format("Wrong yMax: {} vs {}", aabb.yMax, 2.5f));
    _assert(epsilonEqual(aabb.zMax, -2.5f), std::format("Wrong zMax: {} vs {}", aabb.zMax, -2.5f));
    _assert(epsilonEqual(aabb.xMin, 0.5f), std::format("Wrong xMin: {} vs {}", aabb.xMin, 0.5f));
    _assert(epsilonEqual(aabb.yMin, 1.5f), std::format("Wrong yMin: {} vs {}", aabb.yMin, 1.5f));
    _assert(epsilonEqual(aabb.zMin, -3.5f), std::format("Wrong zMin: {} vs {}", aabb.zMin, -3.5f));
    box.localTranslate(-glm::vec3(1, 2, -3));

    println("\t\tAABB TEST -- Rotated box");
    box.localRotateY(glm::radians(45.0f));
    aabb = box.getAABBExtents();
    _assert(epsilonEqual(aabb.xMax, 0.5f * (float)sqrt(2)), std::format("Wrong xMax: {} vs {}", aabb.xMax, 0.5f * (float)sqrt(2)));
    _assert(epsilonEqual(aabb.yMax, 0.5f), std::format("Wrong yMax: {} vs {}", aabb.yMax, 0.5f));
    _assert(epsilonEqual(aabb.zMax, 0.5f * (float)sqrt(2)), std::format("Wrong zMax: {} vs {}", aabb.zMax, 0.5f));
    _assert(epsilonEqual(aabb.xMin, -0.5f * (float)sqrt(2)), std::format("Wrong xMin: {} vs {}", aabb.xMin, -0.5f * (float)sqrt(2)));
    _assert(epsilonEqual(aabb.yMin, -0.5f), std::format("Wrong yMin: {} vs {}", aabb.yMin, -0.5f));
    _assert(epsilonEqual(aabb.zMin, -0.5f * (float)sqrt(2)), std::format("Wrong zMin: {} vs {}", aabb.zMin, -0.5f * (float)sqrt(2)));
    box.localRotateY(glm::radians(-45.0f));

    println("\t\tAABB TEST -- Scaled box");
    box.localScaleAll(glm::vec3(1.0f, 2.0f, 3.0f));
    box.width = 1.5f;
    aabb = box.getAABBExtents();
    _assert(epsilonEqual(aabb.xMax, 0.5f * 1.5f), std::format("Wrong xMax: {} vs {}", aabb.xMax, 0.5f * 1.5f));
    _assert(epsilonEqual(aabb.yMax, 0.5f * 2), std::format("Wrong yMax: {} vs {}", aabb.yMax, 0.5f * 2));
    _assert(epsilonEqual(aabb.zMax, 0.5f * 3), std::format("Wrong zMax: {} vs {}", aabb.zMax, 0.5f * 3));
    _assert(epsilonEqual(aabb.xMin, -0.5f * 1.5f), std::format("Wrong xMin: {} vs {}", aabb.xMin, -0.5f * 1.5f));
    _assert(epsilonEqual(aabb.yMin, -0.5f * 2), std::format("Wrong yMin: {} vs {}", aabb.yMin, -0.5f * 2));
    _assert(epsilonEqual(aabb.zMin, -0.5f * 3), std::format("Wrong zMin: {} vs {}", aabb.zMin, -0.5f * 3));
    box.localScaleAll(glm::vec3(1.0f, 1/2.0f, 1/3.0f));
    box.width = 1.0f;

    println("\t\tPOINTS TEST -- Basic");

    constexpr int expectedCount = POINT_PARAMETER * POINT_PARAMETER * 6 - POINT_PARAMETER * 8;
    PointSet points = box.getPointSet();
    _assert(points.size() >= expectedCount, std::format("Too few points: {} vs {}", points.size(), expectedCount));
    for ( glm::vec3 point : points)
    {
        _assert(box.inBounds(point), std::format("{} Not in bounds", point));
    }

    println("\t\tPOINTS TEST -- Funky");
    box.localTranslate(glm::vec3(1.0f, 2.0f, 3.0f));
    box.localRotateY(glm::radians(45.0f));
    box.localRotateX(glm::radians(-45.0f));
    box.localRotateZ(glm::radians(65.0f));
    box.localScaleAll(glm::vec3(1.0f, 2.0f, 3.0f));
    points = box.getPointSet();
    _assert(points.size() >= expectedCount, std::format("Too few points: {} vs {}", points.size(), expectedCount));

    for (glm::vec3 point : points)
    {
        _assert(box.inBounds(point), std::format("{} Not in bounds", point));
    }

    println("\tEND BOX COLLIDER TEST");
}

void sphereTest()
{
    println("\tSPHERE COLLIDER TEST");
    SphereCollider sphere = SphereCollider();

    println("\t\tBOUNDS TEST -- Basic");

    for (float x = -1; x < 1; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1; z < 1; z += 0.1f) {
                const glm::vec3 p = {x, y, z};
                if (epsilonLessThanEqual(glm::length(p), 1)) {
                    _assert(sphere.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!sphere.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }

    println("\t\tBOUNDS TEST -- Rotated sphere");

    sphere.localRotateY(glm::radians(45.0f));
    sphere.localRotateX(glm::radians(-45.0f));
    sphere.localRotateZ(glm::radians(65.0f));
    for (float x = -1; x < 1; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1; z < 1; z += 0.1f) {
                const glm::vec3 p = {x, y, z};
                if (epsilonLessThanEqual(glm::length(p), 1)) {
                    _assert(sphere.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!sphere.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }
    sphere.localRotateZ((glm::radians(-65.0f)));
    sphere.localRotateX((glm::radians(45.0f)));
    sphere.localRotateY((glm::radians(-45.0f)));

    println("\t\tBOUNDS TEST -- Enlarged sphere");

    sphere.radius = 2.47f;
    for (float x = -3; x < 3; x += 0.1f) {
        for (float y = -3; y < 3; y += 0.1f) {
            for (float z = -3; z < 3; z += 0.1f) {
                const glm::vec3 p = {x, y, z};
                if (epsilonLessThanEqual(glm::length(p), 2.47f)) {
                    _assert(sphere.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!sphere.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }
    sphere.radius = 1;

    println("\t\tBOUNDS TEST -- Non-uniformly scaled sphere");

    glm::vec3 scaling = {2, 1, 1};
    sphere.localScaleAll(scaling);
    for (float x = -3; x < 3; x += 0.1f) {
        for (float y = -1; y < 1; y += 0.1f) {
            for (float z = -1; z < 1; z += 0.1f) {
                const glm::vec3 p = {x, y, z};
                const glm::vec3 p2 = p * (glm::vec3(1) / scaling);
                // Sphere eq: (0.5x)^2 + y^2 + z^2 <= 1
                if (epsilonLessThanEqual(glm::dot(p2, p2), 1)) {
                    _assert(sphere.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!sphere.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }
    sphere.localScaleAll(glm::vec3(1) / scaling);

    println("\t\tBOUNDS TEST -- Translated sphere");

    glm::vec3 move = {2, 4, -5.5f};
    sphere.localTranslate(move);
    for (float x = 0.5f; x < 3.5f; x += 0.1f) {
        for (float y = 2.5f; y < 5.5f; y += 0.1f) {
            for (float z = -7.0f; z < -4.0f; z += 0.1f) {
                const glm::vec3 p = {x, y, z};
                const glm::vec3 p2 = p - move;
                // Sphere eq: (x - 2)^2 + (y - 4)^2 + (z + 5.5)^2 <= 1
                if (epsilonLessThanEqual(glm::dot(p2, p2), 1)) {
                    _assert(sphere.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!sphere.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }
    sphere.localTranslate(-move);

    println("\t\tSPHERE BOUNDS TEST -- Basic");

    SphereBounds sB = sphere.getSphereBounds();
    _assert(epsilonEqual(sB.center, sphere.getGlobalPosition()), std::format("Wrong center: {} vs {}", sB.center, sphere.getGlobalPosition()));
    _assert(epsilonEqual(sB.radius, sphere.radius), std::format("Wrong radius: {} vs {}", sB.radius, sphere.radius));

    println("\t\tSPHERE BOUNDS TEST -- Rotated and translated sphere");

    sphere.localRotateY(glm::radians(45.0f));
    sphere.localRotateX(glm::radians(-86.42));
    sphere.localTranslate({-4, 16.79f, 9});

    sB = sphere.getSphereBounds();
    _assert(epsilonEqual(sB.center, sphere.getGlobalPosition()), std::format("Wrong center: {} vs {}", sB.center, sphere.getGlobalPosition()));
    _assert(epsilonEqual(sB.radius, sphere.radius), std::format("Wrong radius: {} vs {}", sB.radius, sphere.radius));

    sphere.localTranslate({4, -16.79f, -9});
    sphere.localRotateX(glm::radians(86.42));
    sphere.localRotateY(glm::radians(-45.0f));

    println("\t\tSPHERE BOUNDS TEST -- Elongated sphere");

    scaling = {7.36f, 0.67f, 5.42f};
    sphere.localScaleAll(scaling);

    sB = sphere.getSphereBounds();
    _assert(epsilonEqual(sB.center, sphere.getGlobalPosition()), std::format("Wrong center: {} vs {}", sB.center, sphere.getGlobalPosition()));
    _assert(epsilonEqual(sB.radius, sphere.radius * maxComponent(scaling)), std::format("Wrong radius: {} vs {}", sB.radius, sphere.radius * maxComponent(scaling)));

    sphere.localScaleAll(glm::vec3(1) / scaling);

    println("\t\tAABB TEST -- Basic");

    AABBExtents aabb = sphere.getAABBExtents();
    _assert(epsilonEqual(aabb.xMax, 1), std::format("Wrong xMax: {} vs {}", aabb.xMax, 1));
    _assert(epsilonEqual(aabb.yMax, 1), std::format("Wrong yMax: {} vs {}", aabb.yMax, 1));
    _assert(epsilonEqual(aabb.zMax, 1), std::format("Wrong zMax: {} vs {}", aabb.zMax, 1));
    _assert(epsilonEqual(aabb.xMin, -1), std::format("Wrong xMin: {} vs {}", aabb.xMin, -1));
    _assert(epsilonEqual(aabb.yMin, -1), std::format("Wrong yMin: {} vs {}", aabb.yMin, -1));
    _assert(epsilonEqual(aabb.zMin, -1), std::format("Wrong zMin: {} vs {}", aabb.zMin, -1));

    println("\t\tAABB TEST -- Translated sphere");

    move = {16.42f, 5.89f, -73};
    sphere.localTranslate(move);

    aabb = sphere.getAABBExtents();
    _assert(epsilonEqual(aabb.xMax, 1 + move.x), std::format("Wrong xMax: {} vs {}", aabb.xMax, 1 + move.x));
    _assert(epsilonEqual(aabb.yMax, 1 + move.y), std::format("Wrong yMax: {} vs {}", aabb.yMax, 1 + move.y));
    _assert(epsilonEqual(aabb.zMax, 1 + move.z), std::format("Wrong zMax: {} vs {}", aabb.zMax, 1 + move.z));
    _assert(epsilonEqual(aabb.xMin, -1 + move.x), std::format("Wrong xMin: {} vs {}", aabb.xMin, -1 + move.x));
    _assert(epsilonEqual(aabb.yMin, -1 + move.y), std::format("Wrong yMin: {} vs {}", aabb.yMin, -1 + move.y));
    _assert(epsilonEqual(aabb.zMin, -1 + move.z), std::format("Wrong zMin: {} vs {}", aabb.zMin, -1 + move.z));

    sphere.localTranslate(-move);

    println("\t\tAABB TEST -- Rotated sphere");

    sphere.localRotateX(glm::radians(63.15f));
    sphere.localRotateY(glm::radians(-18.54f));
    sphere.localRotateZ(glm::radians(174.77f));

    aabb = sphere.getAABBExtents();
    _assert(epsilonEqual(aabb.xMax, 1), std::format("Wrong xMax: {} vs {}", aabb.xMax, 1));
    _assert(epsilonEqual(aabb.yMax, 1), std::format("Wrong yMax: {} vs {}", aabb.yMax, 1));
    _assert(epsilonEqual(aabb.zMax, 1), std::format("Wrong zMax: {} vs {}", aabb.zMax, 1));
    _assert(epsilonEqual(aabb.xMin, -1), std::format("Wrong xMin: {} vs {}", aabb.xMin, -1));
    _assert(epsilonEqual(aabb.yMin, -1), std::format("Wrong yMin: {} vs {}", aabb.yMin, -1));
    _assert(epsilonEqual(aabb.zMin, -1), std::format("Wrong zMin: {} vs {}", aabb.zMin, -1));

    sphere.localRotateZ(glm::radians(-174.77f));
    sphere.localRotateY(glm::radians(18.54f));
    sphere.localRotateX(glm::radians(-63.15f));

    println("\t\tAABB TEST -- Scaled sphere");

    scaling = {0.46f, 12.73f, 3};
    sphere.localScaleAll(scaling);
    sphere.radius = 2;

    aabb = sphere.getAABBExtents();
    _assert(epsilonEqual(aabb.xMax, scaling.x * 2), std::format("Wrong xMax: {} vs {}", aabb.xMax, scaling.x * 2));
    _assert(epsilonEqual(aabb.yMax, scaling.y * 2), std::format("Wrong yMax: {} vs {}", aabb.yMax, scaling.y * 2));
    _assert(epsilonEqual(aabb.zMax, scaling.z * 2), std::format("Wrong zMax: {} vs {}", aabb.zMax, scaling.z * 2));
    _assert(epsilonEqual(aabb.xMin, -scaling.x * 2), std::format("Wrong xMin: {} vs {}", aabb.xMin,-scaling.x * 2));
    _assert(epsilonEqual(aabb.yMin, -scaling.y * 2), std::format("Wrong yMin: {} vs {}", aabb.yMin,-scaling.y * 2));
    _assert(epsilonEqual(aabb.zMin, -scaling.z * 2), std::format("Wrong zMin: {} vs {}", aabb.zMin,-scaling.z * 2));

    sphere.radius = 1;
    sphere.localScaleAll(glm::vec3(1) / scaling);

    println("\t\tAABB TEST -- Rotated oblong sphere");

    sphere.localRotateY(glm::radians(45.0f));
    sphere.localScaleAll({2, 1, 1});

    //Values found through manual testing in geogebra
    aabb = sphere.getAABBExtents();
    _assert(epsilonEqual(aabb.xMax, 1.581137f), std::format("Wrong xMax: {} vs {}", aabb.xMax, 1.581137f));
    _assert(epsilonEqual(aabb.yMax, 1), std::format("Wrong yMax: {} vs {}", aabb.yMax, 1));
    _assert(epsilonEqual(aabb.zMax, 1.581137f), std::format("Wrong zMax: {} vs {}", aabb.zMax, 1.581137f));
    _assert(epsilonEqual(aabb.xMin, -1.581137f), std::format("Wrong xMin: {} vs {}", aabb.xMin, -1.581137f));
    _assert(epsilonEqual(aabb.yMin, -1), std::format("Wrong yMin: {} vs {}", aabb.yMin, -1));
    _assert(epsilonEqual(aabb.zMin, -1.581137f), std::format("Wrong zMin: {} vs {}", aabb.zMin, -1.581137f));

    sphere.localRotateY(glm::radians(-45.0f));
    sphere.localScaleAll({0.5f, 1, 1});

    println("\t\tPOINTS TEST -- Basic");

    constexpr int expectedCount = POINT_PARAMETER * 7 + 2;
    PointSet points = sphere.getPointSet();
    _assert(points.size() >= expectedCount, std::format("Too few points: {} vs {}", points.size(), expectedCount));
    for ( glm::vec3 point : points)
    {
        _assert(sphere.inBounds(point), std::format("{} Not in bounds", point));
    }

    println("\t\tPOINTS TEST -- Funky");

    sphere.localTranslate(glm::vec3(1.0f, 2.0f, 3.0f));
    sphere.localRotateY(glm::radians(45.0f));
    sphere.localRotateX(glm::radians(-45.0f));
    sphere.localRotateZ(glm::radians(65.0f));
    sphere.localScaleAll(glm::vec3(1.0f, 2.0f, 3.0f));
    points = sphere.getPointSet();
    _assert(points.size() >= expectedCount, std::format("Too few points: {} vs {}", points.size(), expectedCount));

    for (glm::vec3 point : points)
    {
        _assert(sphere.inBounds(point), std::format("{} Not in bounds", point));
    }

    println("\tEND SPHERE COLLIDER TEST");
}

void capsuleTest()
{
    println("\tCAPSULE COLLIDER TEST");
    println("\t\tBOUNDS TEST -- Basic");
    println("\t\tBOUNDS TEST -- Rotated capsule");
    println("\t\tBOUNDS TEST -- Elongated capsule");
    println("\t\tBOUNDS TEST -- Non-uniformly scaled capsule");
    println("\t\tBOUNDS TEST -- Translated capsule");
    println("\t\tSPHERE BOUNDS TEST -- Basic");
    println("\t\tSPHERE BOUNDS TEST -- Rotated and translated capsule");
    println("\t\tSPHERE BOUNDS TEST -- Elongated capsule");
    println("\t\tAABB TEST -- Basic");
    println("\t\tAABB TEST -- Translated capsule");
    println("\t\tAABB TEST -- Rotated capsule");
    println("\t\tAABB TEST -- Scaled capsule");
    println("\t\tPOINTS TEST -- Basic");
    println("\t\tPOINTS TEST -- Funky");
    println("\tEND CAPSULE COLLIDER TEST");
    println("COLLIDER TEST");
    println("END COLLIDER TEST");
}

void coneTest()
{
    println("\tCONE COLLIDER TEST");
    println("\t\tBOUNDS TEST -- Basic");
    println("\t\tBOUNDS TEST -- Rotated cone");
    println("\t\tBOUNDS TEST -- Elongated cone");
    println("\t\tBOUNDS TEST -- Non-uniformly scaled cone");
    println("\t\tBOUNDS TEST -- Translated cone");
    println("\t\tSPHERE BOUNDS TEST -- Basic");
    println("\t\tSPHERE BOUNDS TEST -- Rotated and translated cone");
    println("\t\tSPHERE BOUNDS TEST -- Elongated cone");
    println("\t\tAABB TEST -- Basic");
    println("\t\tAABB TEST -- Translated cone");
    println("\t\tAABB TEST -- Rotated cone");
    println("\t\tAABB TEST -- Scaled cone");
    println("\t\tPOINTS TEST -- Basic");
    println("\t\tPOINTS TEST -- Funky");
    println("\tEND CONE COLLIDER TEST");
    println("COLLIDER TEST");
    println("END COLLIDER TEST");
}

int main() {
    println("COLLIDER TEST");
    boxTest();
    sphereTest();
    println("END COLLIDER TEST");
}

