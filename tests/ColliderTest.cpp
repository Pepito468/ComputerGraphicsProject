#include <cmath>
#include <format>
#include "Collider.hpp"
#include "Debug.hpp"
#include "Relations.hpp"
#include <glm/glm.hpp>

void inBoundsCheck(const Collider& coll, const glm::vec3 from, const glm::vec3 to, const std::function<bool(glm::vec3)>& condition)
{
    for (float x = from.x; x < to.x; x += 0.1f) {
        for (float y = from.y; y < to.y; y += 0.1f) {
            for (float z = from.z; z < to.z; z += 0.1f) {
                const glm::vec3 p = {x, y, z};
                if (condition(p)) {
                    _assert(coll.inBounds(p), std::format("{} Not in bounds", p));
                } else {
                    _assert(!coll.inBounds(p), std::format("{} In bounds", p));
                }
            }
        }
    }
}

void sphereCheck(const SphereBounds& sB, const glm::vec3 expectedCenter, float expectedRadius)
{
    _assert(epsilonEqual(sB.center, expectedCenter), std::format("Wrong center: {} vs {}", sB.center, expectedCenter));
    _assert(epsilonGreaterThanEqual(sB.radius, expectedRadius), std::format("Wrong radius: {} vs {}", sB.radius, expectedRadius));
}

void AABBCheck(const AABBExtents& aabb, const glm::vec3 max, const glm::vec3 min)
{
    _assert(epsilonEqual(aabb.xMax, max.x), std::format("Wrong xMax: {} vs {}", aabb.xMax, max.x));
    _assert(epsilonEqual(aabb.yMax, max.y), std::format("Wrong yMax: {} vs {}", aabb.yMax, max.y));
    _assert(epsilonEqual(aabb.zMax, max.z), std::format("Wrong zMax: {} vs {}", aabb.zMax, max.z));
    _assert(epsilonEqual(aabb.xMin, min.x), std::format("Wrong xMin: {} vs {}", aabb.xMin, min.x));
    _assert(epsilonEqual(aabb.yMin, min.y), std::format("Wrong yMin: {} vs {}", aabb.yMin, min.y));
    _assert(epsilonEqual(aabb.zMin, min.z), std::format("Wrong zMin: {} vs {}", aabb.zMin, min.z));
}

void pointsCheck(const Collider& coll, const int expectedCount)
{
    const PointSet points = coll.getPointSet();
    _assert(points.size() >= expectedCount, std::format("Too few points: {} vs {}", points.size(), expectedCount));
    for ( glm::vec3 point : points)
    {
        _assert(coll.inBounds(point), std::format("{} Not in bounds", point));
    }
}

void boxTest() {
    println("\tBOX COLLIDER TEST");
    BoxCollider box = BoxCollider();

    println("\t\tBOUNDS TEST -- Basic");
    inBoundsCheck(box, {-1, -1, -1}, {1, 1, 1},
        [] (const glm::vec3 p)
        {
            constexpr glm::vec3 limits = glm::vec3(0.5f);
            return epsilonLessThanEqual(glm::abs(p), limits);
        });

    println("\t\tBOUNDS TEST -- Rotated box");
    box.localRotateY(glm::radians(45.0f));
    inBoundsCheck(box, {-1, -1, -1}, {1, 1, 1},
        [] (const glm::vec3 p)
        {
            const double d = 0.5f * sqrt(2.0f);
            return epsilonBetween(std::abs(p.x) - d, p.z, -std::abs(p.x) + d)
                    && epsilonLessThanEqual(std::abs(p.y), 0.5f);
        });
    box.localRotateY(glm::radians(-45.0f));

    println("\t\tBOUNDS TEST -- Elongated box");
    box.width = 2;
    inBoundsCheck(box, {-1.5f, -1, -1}, {1.5f, 1, 1},
        [] (const glm::vec3 p)
        {
            constexpr glm::vec3 limits = {1, 0.5f, 0.5f};
            return epsilonLessThanEqual(glm::abs(p), limits);
        });

    println("\t\tBOUNDS TEST -- Non-uniformly scaled box");
    box.localScaleAll(glm::vec3(1, 1, 2));
    inBoundsCheck(box, {-1.5f, -1, -1.5}, {1.5f, 1, 1.5},
        [] (const glm::vec3 p)
        {
            constexpr glm::vec3 limits = {1, 0.5f, 1};
            return epsilonLessThanEqual(glm::abs(p), limits);
        });
    box.localScaleAll(glm::vec3(1, 1, 0.5f));
    box.width = 1;

    println("\t\tBOUNDS TEST -- Translated box");
    box.localTranslate(glm::vec3(0.5f));
    inBoundsCheck(box, {-0.5f, -0.5f, -0.5f}, {1.5f, 1.5f, 1.5f},
        [] (const glm::vec3 p)
        {
            return epsilonBetween(VEC3_ZERO, p, VEC3_ONE);
        });
    box.localTranslate(glm::vec3(-0.5f));

    println("\t\tSPHERE BOUNDS TEST -- Basic");
    glm::vec3 v = glm::vec3(box.width, box.height, box.depth) * box.getGlobalScale()* glm::vec3(0.5f);
    float expectedR = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    sphereCheck(box.getSphereBounds(), VEC3_ZERO, expectedR);

    println("\t\tSPHERE BOUNDS TEST -- Rotated and translated box");
    //Radius shouldn't change
    box.localRotateY(glm::radians(45.0f));
    box.localRotateX(glm::radians(-64.0f));
    box.localTranslate(glm::vec3(18, 46, -0.57f));
    sphereCheck(box.getSphereBounds(), {18, 46, -0.57f}, expectedR);
    box.localTranslate(-glm::vec3(18, 46, -0.57f));
    box.localRotateX(glm::radians(64.0f));
    box.localRotateY(glm::radians(-45.0f));

    println("\t\tSPHERE BOUNDS TEST -- Elongated box");
    box.width = 1.7f;
    box.depth = 0.45f;
    box.localScaleAll(glm::vec3(16.3f, 0.42f, 61.54f));
    v = glm::vec3(box.width, box.height, box.depth) * box.getGlobalScale ()* glm::vec3(0.5f);
    expectedR = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    sphereCheck(box.getSphereBounds(), VEC3_ZERO, expectedR);
    box.width = 1;
    box.depth = 1;
    box.localScaleAll(glm::vec3(1/16.3f, 1/0.42f, 1/61.54f));

    println("\t\tAABB TEST -- Basic");
    AABBCheck(box.getAABBExtents(), {0.5f, 0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f});

    println("\t\tAABB TEST -- Translated box");
    box.localTranslate(glm::vec3(1, 2, -3));
    AABBCheck(box.getAABBExtents(), {1.5f, 2.5f, -2.5f}, {0.5f, 1.5f, -3.5f});
    box.localTranslate(-glm::vec3(1, 2, -3));

    println("\t\tAABB TEST -- Rotated box");
    box.localRotateY(glm::radians(45.0f));
    const float invSqrt2 = 0.5f * sqrt(2);
    AABBCheck(box.getAABBExtents(), {invSqrt2, 0.5f, invSqrt2}, {-invSqrt2, -0.5f, -invSqrt2});
    box.localRotateY(glm::radians(-45.0f));

    println("\t\tAABB TEST -- Scaled box");
    box.localScaleAll(glm::vec3(1.0f, 2.0f, 3.0f));
    box.width = 1.5f;
    AABBCheck(box.getAABBExtents(), 0.5f * glm::vec3(1.5f, 2, 3), -0.5f * glm::vec3(1.5f, 2, 3));
    box.localScaleAll(glm::vec3(1.0f, 1/2.0f, 1/3.0f));
    box.width = 1.0f;

    println("\t\tPOINTS TEST -- Basic");

    constexpr int expectedCount = POINT_PARAMETER * POINT_PARAMETER * 6 - POINT_PARAMETER * 8;
    pointsCheck(box, expectedCount);

    println("\t\tPOINTS TEST -- Funky");
    box.localTranslate(glm::vec3(1.0f, 2.0f, 3.0f));
    box.localRotateY(glm::radians(45.0f));
    box.localRotateX(glm::radians(-45.0f));
    box.localRotateZ(glm::radians(65.0f));
    box.localScaleAll(glm::vec3(1.0f, 2.0f, 3.0f));
    pointsCheck(box, expectedCount);

    println("\tEND BOX COLLIDER TEST");
}

void sphereTest()
{
    println("\tSPHERE COLLIDER TEST");
    SphereCollider sphere = SphereCollider();

    println("\t\tBOUNDS TEST -- Basic");
    inBoundsCheck(sphere, {-1, -1, -1}, {1, 1, 1},
        [](const glm::vec3 p)
        {
            return epsilonLessThanEqual(glm::length(p), 1);
        });

    println("\t\tBOUNDS TEST -- Rotated sphere");
    sphere.localRotateY(glm::radians(45.0f));
    sphere.localRotateX(glm::radians(-45.0f));
    sphere.localRotateZ(glm::radians(65.0f));
    inBoundsCheck(sphere, {-1, -1, -1}, {1, 1, 1},
        [](const glm::vec3 p)
        {
            return epsilonLessThanEqual(glm::length(p), 1);
        });
    sphere.localRotateZ((glm::radians(-65.0f)));
    sphere.localRotateX((glm::radians(45.0f)));
    sphere.localRotateY((glm::radians(-45.0f)));

    println("\t\tBOUNDS TEST -- Enlarged sphere");
    sphere.radius = 2.47f;
    inBoundsCheck(sphere, {-3, -3, -3}, {3, 3, 3},
        [](const glm::vec3 p)
        {
            return epsilonLessThanEqual(glm::length(p), 2.47f);
        });
    sphere.radius = 1;

    println("\t\tBOUNDS TEST -- Non-uniformly scaled sphere");
    glm::vec3 scaling = {2, 1, 1};
    sphere.localScaleAll(scaling);
    inBoundsCheck(sphere, {-3, -1, -1}, {3, 1, 1},
        [scaling](glm::vec3 p)
        {
            // Sphere eq: (0.5x)^2 + y^2 + z^2 <= 1
            p *= VEC3_ONE / scaling;
            return epsilonLessThanEqual(glm::dot(p, p), 1);
        });
    sphere.localScaleAll(VEC3_ONE / scaling);

    println("\t\tBOUNDS TEST -- Translated sphere");
    glm::vec3 move = {2, 4, -5.5f};
    sphere.localTranslate(move);
    inBoundsCheck(sphere, {0.5f, 2.5f, -7}, {3.5f, 5.5f, -4},
        [move](glm::vec3 p)
        {
            // Sphere eq: (x - 2)^2 + (y - 4)^2 + (z + 5.5)^2 <= 1
            p -= move;
            return epsilonLessThanEqual(glm::dot(p, p), 1);
        });
    sphere.localTranslate(-move);

    println("\t\tSPHERE BOUNDS TEST -- Basic");
    sphereCheck(sphere.getSphereBounds(), VEC3_ZERO, 1);

    println("\t\tSPHERE BOUNDS TEST -- Rotated and translated sphere");
    sphere.localRotateY(glm::radians(45.0f));
    sphere.localRotateX(glm::radians(-86.42));
    sphere.localTranslate({-4, 16.79f, 9});
    sphereCheck(sphere.getSphereBounds(), {-4, 16.79f, 9}, 1);
    sphere.localTranslate({4, -16.79f, -9});
    sphere.localRotateX(glm::radians(86.42));
    sphere.localRotateY(glm::radians(-45.0f));

    println("\t\tSPHERE BOUNDS TEST -- Elongated sphere");
    scaling = {7.36f, 0.67f, 5.42f};
    sphere.localScaleAll(scaling);
    sphereCheck(sphere.getSphereBounds(), VEC3_ZERO, maxComponent(scaling));
    sphere.localScaleAll(glm::vec3(1) / scaling);

    println("\t\tAABB TEST -- Basic");
    AABBCheck(sphere.getAABBExtents(), {1, 1, 1}, {-1, -1, -1});

    println("\t\tAABB TEST -- Translated sphere");
    move = {16.42f, 5.89f, -73};
    sphere.localTranslate(move);
    AABBCheck(sphere.getAABBExtents(), VEC3_ONE + move, -VEC3_ONE + move);
    sphere.localTranslate(-move);

    println("\t\tAABB TEST -- Rotated sphere");
    sphere.localRotateX(glm::radians(63.15f));
    sphere.localRotateY(glm::radians(-18.54f));
    sphere.localRotateZ(glm::radians(174.77f));
    AABBCheck(sphere.getAABBExtents(), {1, 1, 1}, {-1, -1, -1});
    sphere.localRotateZ(glm::radians(-174.77f));
    sphere.localRotateY(glm::radians(18.54f));
    sphere.localRotateX(glm::radians(-63.15f));

    println("\t\tAABB TEST -- Scaled sphere");
    scaling = {0.46f, 12.73f, 3};
    sphere.localScaleAll(scaling);
    sphere.radius = 2;
    AABBCheck(sphere.getAABBExtents(), scaling * 2.0f, -scaling * 2.0f);
    sphere.radius = 1;
    sphere.localScaleAll(VEC3_ONE / scaling);

    println("\t\tAABB TEST -- Rotated oblong sphere");
    sphere.localRotateY(glm::radians(45.0f));
    sphere.localScaleAll({2, 1, 1});
    //Values found through manual testing in geogebra
    AABBCheck(sphere.getAABBExtents(), {1.581137f, 1, 1.581137f}, {-1.581137f, -1, -1.581137f});
    sphere.localRotateY(glm::radians(-45.0f));
    sphere.localScaleAll({0.5f, 1, 1});

    println("\t\tPOINTS TEST -- Basic");
    constexpr int expectedCount = POINT_PARAMETER * 7 + 2;
    pointsCheck(sphere, expectedCount);

    println("\t\tPOINTS TEST -- Funky");
    sphere.localTranslate(glm::vec3(1.0f, 2.0f, 3.0f));
    sphere.localRotateY(glm::radians(45.0f));
    sphere.localRotateX(glm::radians(-45.0f));
    sphere.localRotateZ(glm::radians(65.0f));
    sphere.localScaleAll(glm::vec3(1.0f, 2.0f, 3.0f));
    pointsCheck(sphere, expectedCount);

    println("\tEND SPHERE COLLIDER TEST");
}

void capsuleTest()
{
    println("\tCAPSULE COLLIDER TEST");
    CapsuleCollider cap = CapsuleCollider();

    println("\t\tBOUNDS TEST -- Basic");
    inBoundsCheck(cap, {-1.5f, -2, -1.5f}, {1.5f, 2, 1.5f},
        [](const glm::vec3 p)
        {
            if (-0.5f <= p.y && p.y <= 0.5f)
                return epsilonLessThanEqual(p.x * p.x + p.z * p.z, 1);

            return epsilonLessThanEqual(glm::distance(p, VEC3_Y * 0.5f * glm::sign(p.y)), 1);
        });

    println("\t\tBOUNDS TEST -- Rotated capsule");
    cap.globalRotateZ(glm::radians(90.0f));
    inBoundsCheck(cap, {-2, -1.5f, -1.5f}, {2, -1.5f, 1.5f},
        [](const glm::vec3 p)
        {
            if (-0.5f <= p.x && p.x <= 0.5f)
                return epsilonLessThanEqual(p.y * p.y + p.z * p.z, 1);

            return epsilonLessThanEqual(glm::distance(p, VEC3_Z * 0.5f * glm::sign(p.x)), 1);
        });
    cap.globalRotateZ(glm::radians(-90.0f));

    println("\t\tBOUNDS TEST -- Elongated capsule");
    cap.height = 1.5f;
    cap.radius = 2;
    inBoundsCheck(cap, {-2.5f, -3, -2.5f}, {2.5f, 3, 2.5f},
        [](const glm::vec3 p)
        {
            if (-0.75f <= p.y && p.y <= 0.75f)
                return epsilonLessThanEqual(p.x * p.x + p.z * p.z, 4);

            return epsilonLessThanEqual(glm::distance(p, VEC3_Y * 0.75f * glm::sign(p.y)), 2);
        });

    println("\t\tBOUNDS TEST -- Non-uniformly scaled capsule");
    glm::vec3 scaling = {1, 1, 0.85f};
    cap.localScaleAll(scaling);
    inBoundsCheck(cap, {-2.5f, -3, -2.5f}, {2.5f, 3, 2.5f},
        [scaling](glm::vec3 p)
        {
            p /= scaling;
            if (-0.75f <= p.y && p.y <= 0.75f)
                return epsilonLessThanEqual(p.x * p.x + p.z * p.z, 4);

            return epsilonLessThanEqual(glm::distance(p, VEC3_Y * 0.75f * glm::sign(p.y)), 2);
        });
    cap.localScaleAll(VEC3_ONE / scaling);
    cap.height = 1;
    cap.radius = 1;

    println("\t\tBOUNDS TEST -- Translated capsule");
    glm::vec3 move = {-4.56f, 34, 0.765f};
    cap.globalTranslate(move);
    inBoundsCheck(cap, move + glm::vec3(-1.5f, -2, -1.5f), move + glm::vec3(1.5f, 2, 1.5f),
        [move](glm::vec3 p)
        {
            p -= move;
            if (-0.5f <= p.y && p.y <= 0.5f)
                return epsilonLessThanEqual(p.x * p.x + p.z * p.z, 1);

            return epsilonLessThanEqual(glm::distance(p, VEC3_Y * 0.5f * glm::sign(p.y)), 1);
        });
    cap.globalTranslate(-move);

    println("\t\tSPHERE BOUNDS TEST -- Basic");
    sphereCheck(cap.getSphereBounds(), VEC3_ZERO, 1.5f);

    println("\t\tSPHERE BOUNDS TEST -- Rotated and translated capsule");
    move = {1.43f, 7.893f, -34};
    cap.globalTranslate(move);
    cap.globalRotateZ(glm::radians(-34.56f));
    cap.globalRotateY(glm::radians(75.43f));
    sphereCheck(cap.getSphereBounds(), move, 1.5f);
    cap.globalRotateY(glm::radians(-75.43f));
    cap.globalRotateZ(glm::radians(34.56f));
    cap.globalTranslate(-move);

    println("\t\tSPHERE BOUNDS TEST -- Elongated capsule");
    scaling = {0.75f, 2, 6};
    cap.globalScaleAll(scaling);
    sphereCheck(cap.getSphereBounds(), VEC3_ZERO, 6);
    cap.globalScaleAll(VEC3_ONE / scaling);

    println("\t\tAABB TEST -- Basic");
    AABBCheck(cap.getAABBExtents(), {1, 1.5f, 1}, {-1, -1.5f, -1});

    println("\t\tAABB TEST -- Translated capsule");
    move = {34, -7.5470f, 757};
    cap.globalTranslate(move);
    AABBCheck(cap.getAABBExtents(), move + glm::vec3(1, 1.5f, 1), move - glm::vec3(1, 1.5f, 1));
    cap.globalTranslate(-move);

    println("\t\tAABB TEST -- Rotated capsule");
    cap.globalRotateZ(glm::radians(90.0f));
    AABBCheck(cap.getAABBExtents(), {1.5f, 1, 1}, {-1.5f, -1, -1});
    cap.globalRotateZ(glm::radians(-90.0f));

    println("\t\tAABB TEST -- Scaled capsule");
    scaling = {4.4f, 5, 0.56f};
    cap.globalScaleAll(scaling);
    AABBCheck(cap.getAABBExtents(), glm::vec3(1, 1.5f, 1) * scaling, -glm::vec3(1, 1.5f, 1) * scaling);
    cap.globalScaleAll(VEC3_ONE / scaling);

    println("\t\tPOINTS TEST -- Basic");
    constexpr int expectedPoints = 16 * POINT_PARAMETER + 18;
    pointsCheck(cap, expectedPoints);

    println("\t\tPOINTS TEST -- Funky");
    cap.globalTranslate({12.43f, -43, 0.543f});
    cap.globalRotateZ(glm::radians(234.0836f));
    cap.globalRotateY(glm::radians(-34.9372f));
    cap.globalScaleAll({32, 0.343f, 12});
    pointsCheck(cap, expectedPoints);

    println("\tEND CAPSULE COLLIDER TEST");
}

void coneTest()
{
    println("\tCONE COLLIDER TEST");
    ConeCollider cone = ConeCollider(1.0f, glm::radians(45.0f));

    println("\t\tBOUNDS TEST -- Basic");
    inBoundsCheck(cone, {-1, -1, -0.2f}, {1, 1, 1.2f},
        [] (const glm::vec3 p)
        {
            if (!epsilonLessThanEqual(glm::length(p), 1)) return false;
            //log(std::format("p: {}, l/d: {}/{}", p, glm::length(p), std::acos(glm::dot(VEC3_Z, normalize(p)))));
            return epsilonLessThanEqual(std::acos(glm::dot(VEC3_Z, normalize(p))), glm::radians(45.0f));
        });

    println("\t\tBOUNDS TEST -- Rotated cone");
    cone.globalRotateY(glm::radians(90.0f));
    inBoundsCheck(cone, {-0.2f, -1, -1}, {1.2f, 1, 1},
        [] (const glm::vec3 p)
        {
            if (!epsilonLessThanEqual(glm::length(p), 1)) return false;

            return epsilonLessThanEqual(std::acos(glm::dot(VEC3_X, normalize(p))), glm::radians(45.0f));
        });
    cone.globalRotateY(glm::radians(-90.0f));

    println("\t\tBOUNDS TEST -- Elongated cone");
    cone.radius = 2;
    cone.aperture = glm::radians(30.0f);
    inBoundsCheck(cone, {-1, -1, -0.2f}, {1, 1, 2.2f},
        [] (const glm::vec3 p)
        {
            if (!epsilonLessThanEqual(glm::length(p), 2)) return false;

            return epsilonLessThanEqual(std::acos(glm::dot(VEC3_Z, normalize(p))), glm::radians(30.0f));
        });

    println("\t\tBOUNDS TEST -- Non-uniformly scaled cone");
    glm::vec3 scaling = {2, 1.4f, 0.654f};
    cone.globalScaleAll(scaling);
    inBoundsCheck(cone, {-2, -1.4, -0.1308f}, {2, 1.4f, 1.4388f},
        [scaling] (glm::vec3 p)
        {
            p /= scaling;
            if (!epsilonLessThanEqual(glm::length(p), 2)) return false;

            return epsilonLessThanEqual(std::acos(glm::dot(VEC3_Z, normalize(p))), glm::radians(30.0f));
        });
    cone.globalScaleAll(VEC3_ONE / scaling);
    cone.radius = 1;
    cone.aperture = glm::radians(45.0f);

    println("\t\tBOUNDS TEST -- Translated cone");
    glm::vec3 move = {2, -43, 12.434f};
    cone.globalTranslate(move);
    inBoundsCheck(cone, move + glm::vec3(-1, -1, -0.2f), move - glm::vec3(1, 1, 1.2f),
        [move] (glm::vec3 p)
        {
            p -= move;
            if (!epsilonLessThanEqual(glm::length(p), 1)) return false;

            return epsilonLessThanEqual(std::acos(glm::dot(VEC3_Z, normalize(p))), glm::radians(45.0f));
        });
    cone.globalTranslate(-move);

    println("\t\tSPHERE BOUNDS TEST -- Basic");
    //Radius value found through manual testing in geogebra
    sphereCheck(cone.getSphereBounds(), {0, 0, 0.5f}, 0.73484f);

    println("\t\tSPHERE BOUNDS TEST -- Rotated and translated cone");
    move = {21, -322.321f, 32};
    cone.globalTranslate(move);
    cone.globalRotateX(glm::radians(-90.0f));
    sphereCheck(cone.getSphereBounds(), glm::vec3(0, 0.5f, 0) + move, 0.73484f);
    cone.globalRotateX(glm::radians(90.0f));
    cone.globalTranslate(-move);

    println("\t\tSPHERE BOUNDS TEST -- Elongated cone");
    cone.radius = 4;
    cone.aperture = glm::radians(15.0f);
    sphereCheck(cone.getSphereBounds(), {0, 0, 2}, 2.13194f);
    cone.radius = 1;
    cone.aperture = glm::radians(45.0f);

    println("\t\tAABB TEST -- Basic");
    //Values found through testing in geogebra
    constexpr glm::vec3 baseMax = {0.707106f, 0.707106f, 1};
    constexpr glm::vec3 baseMin = {-0.707106f, -0.707106f, 0};
    AABBCheck(cone.getAABBExtents(), baseMax, baseMin);

    println("\t\tAABB TEST -- Translated cone");
    move = {12.49f, -9.32f, 3};
    cone.globalTranslate(move);
    AABBCheck(cone.getAABBExtents(), baseMax + move, baseMin + move);
    cone.globalTranslate(-move);

    println("\t\tAABB TEST -- Rotated cone");
    cone.globalRotateY(glm::radians(90.0f));
    AABBCheck(cone.getAABBExtents(), {baseMax.z, baseMax.y, baseMax.x}, {baseMin.z, baseMin.y, baseMin.x});
    cone.globalRotateY(glm::radians(-90.0f));

    println("\t\tAABB TEST -- Scaled cone");
    scaling = {0.43f, 23, 8.432f};
    cone.globalScaleAll(scaling);
    AABBCheck(cone.getAABBExtents(), baseMax * scaling, baseMin * scaling);
    cone.globalScaleAll(VEC3_ONE / scaling);

    println("\t\tPOINTS TEST -- Basic");
    constexpr int expectedPoints = POINT_PARAMETER * (POINT_PARAMETER - 1 + 3);
    pointsCheck(cone, expectedPoints);

    println("\t\tPOINTS TEST -- Funky");
    cone.radius = 3.23f;
    cone.aperture = glm::radians(64.123f);
    cone.globalTranslate({23.32f, 3, -3.432f});
    cone.globalScaleAll({3, 0.432f, 3});
    cone.globalRotateY(glm::radians(43.0f));
    cone.globalRotateZ(glm::radians(34.213f));
    pointsCheck(cone, expectedPoints);

    println("\tEND CONE COLLIDER TEST");
}

int main() {
    println("COLLIDER TEST");
    boxTest();
    sphereTest();
    capsuleTest();
    coneTest();
    println("END COLLIDER TEST");
}

