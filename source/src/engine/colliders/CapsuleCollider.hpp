#ifndef ENGINE_CAPSULECOLLIDER_HPP
#define ENGINE_CAPSULECOLLIDER_HPP

#include "Collider.hpp"

/// A collider shaped like a cylinder with two hemispheres at the ends
class CapsuleCollider: public Collider
{
    float halfHeight() const {return height / 2.0f;}

public:
    /// The radius of the capsule
    BoundFloat radius = PositiveFloat(1.0f);
    /// The length of the capsule's cylinder along the Y-axis
    BoundFloat height = PositiveFloat(1.0f);

    CapsuleCollider() : Collider() {}
    CapsuleCollider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(position, rotation, scale) {}
    CapsuleCollider(const bool isTrigger, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(isTrigger, position, rotation, scale) {}
    CapsuleCollider(const float radius, const float height) : Collider()
    {
        this->radius = radius;
        this->height = height;
    }

    bool inBounds(const glm::vec3 point) const override
    {
        const glm::vec3 p = toLocalSpace(point);
        if (epsilonBetween(-halfHeight(), p.y, halfHeight()))
        {
            //Cylindrical
            return epsilonLessThanEqual((p.x * p.x + p.z * p.z), radius * radius);
        }

        //Sphere caps
        const glm::vec3 capC = VEC3_Y * halfHeight() * glm::sign(p.y);
        return epsilonLessThanEqual(glm::distance(p, capC), radius);
    }

    PointSet getPointSet() const override
    {
        PointSet res = PointSet();

        //Add points along the cylinder
        const float cos30Rad = radius * std::cos(M_PI / 6.0f);
        const float sin30Rad = radius * std::sin(M_PI / 6.0f);
        const float cos45Rad = radius * std::cos(M_PI / 4.0f);
        const float dy = height / POINT_PARAMETER;
        for (int i = 0; i <= POINT_PARAMETER; i++)
        {
            const float y = -halfHeight() + dy * i;
            res.insert(toGlobalSpace({(float)radius, y, 0}));
            res.insert(toGlobalSpace({-radius, y, 0}));
            res.insert(toGlobalSpace({0, y, (float)radius}));
            res.insert(toGlobalSpace({0, y, -(float)radius}));
            res.insert(toGlobalSpace({cos30Rad, y, sin30Rad}));
            res.insert(toGlobalSpace({cos30Rad, y, -sin30Rad}));
            res.insert(toGlobalSpace({-cos30Rad, y, sin30Rad}));
            res.insert(toGlobalSpace({-cos30Rad, y, -sin30Rad}));
            res.insert(toGlobalSpace({sin30Rad, y, cos30Rad}));
            res.insert(toGlobalSpace({sin30Rad, y, -cos30Rad}));
            res.insert(toGlobalSpace({-sin30Rad, y, cos30Rad}));
            res.insert(toGlobalSpace({-sin30Rad, y, -cos30Rad}));
            res.insert(toGlobalSpace({cos45Rad, y, cos45Rad}));
            res.insert(toGlobalSpace({cos45Rad, y, -cos45Rad}));
            res.insert(toGlobalSpace({-cos45Rad, y, cos45Rad}));
            res.insert(toGlobalSpace({-cos45Rad, y, -cos45Rad}));
        }

        //Add points along sphere caps
        const float sqrt3rdRad = std::sqrt(1 / 3.0f) * radius;
        res.insert(toGlobalSpace({0, halfHeight() + radius, 0}));
        res.insert(toGlobalSpace({cos45Rad, halfHeight() + cos45Rad, 0}));
        res.insert(toGlobalSpace({-cos45Rad, halfHeight() + cos45Rad, 0}));
        res.insert(toGlobalSpace({0, halfHeight() + cos45Rad, cos45Rad}));
        res.insert(toGlobalSpace({0, halfHeight() + cos45Rad, -cos45Rad}));
        res.insert(toGlobalSpace({sqrt3rdRad, halfHeight() + sqrt3rdRad, sqrt3rdRad}));
        res.insert(toGlobalSpace({sqrt3rdRad, halfHeight() + sqrt3rdRad, -sqrt3rdRad}));
        res.insert(toGlobalSpace({-sqrt3rdRad, halfHeight() + sqrt3rdRad, sqrt3rdRad}));
        res.insert(toGlobalSpace({-sqrt3rdRad, halfHeight() + sqrt3rdRad, -sqrt3rdRad}));
        res.insert(toGlobalSpace({0, -(halfHeight() + radius), 0}));
        res.insert(toGlobalSpace({cos45Rad, -(halfHeight() + cos45Rad), 0}));
        res.insert(toGlobalSpace({-cos45Rad, -(halfHeight() + cos45Rad), 0}));
        res.insert(toGlobalSpace({0, -(halfHeight() + cos45Rad), cos45Rad}));
        res.insert(toGlobalSpace({0, -(halfHeight() + cos45Rad), -cos45Rad}));
        res.insert(toGlobalSpace({sqrt3rdRad, -(halfHeight() + sqrt3rdRad), sqrt3rdRad}));
        res.insert(toGlobalSpace({sqrt3rdRad, -(halfHeight() + sqrt3rdRad), -sqrt3rdRad}));
        res.insert(toGlobalSpace({-sqrt3rdRad, -(halfHeight() + sqrt3rdRad), sqrt3rdRad}));
        res.insert(toGlobalSpace({-sqrt3rdRad, -(halfHeight() + sqrt3rdRad), -sqrt3rdRad}));

        return res;
    }

    SphereBounds getSphereBounds() const override
    {
        const float hRad = radius * std::max(getGlobalScale().x, getGlobalScale().z);
        const float vRad = (halfHeight() + radius) * getGlobalScale().y;
        return SphereBounds(getGlobalPosition(), std::max(hRad, vRad));
    }

    AABBExtents getAABBExtents() const override
    {
        AABBExtents res = AABBExtents();

        const AABBExtents northCapExt = SphereCollider::getSphereExtents(radius,
            glm::translate(MAT4_I, getLocalYAxis() * halfHeight() * getGlobalScale().y) * getGlobalMatrix());
        const AABBExtents southCapExt = SphereCollider::getSphereExtents(radius,
            glm::translate(MAT4_I, -getLocalYAxis() * halfHeight() * getGlobalScale().y) * getGlobalMatrix());

        res.xMax = std::max(northCapExt.xMax, southCapExt.xMax);
        res.yMax = std::max(northCapExt.yMax, southCapExt.yMax);
        res.zMax = std::max(northCapExt.zMax, southCapExt.zMax);
        res.xMin = std::min(northCapExt.xMin, southCapExt.xMin);
        res.yMin = std::min(northCapExt.yMin, southCapExt.yMin);
        res.zMin = std::min(northCapExt.zMin, southCapExt.zMin);

        return res;
    }

    static CapsuleCollider* fromJSON(const nlohmann::json& json, CapsuleCollider* node = nullptr) {
        CapsuleCollider *newNode = node ? node: new CapsuleCollider();

        newNode->collFromJSON(json);

        if (json.contains("radius")) newNode->radius = json.at("radius").get<float>();
        if (json.contains("height")) newNode->height = json.at("height").get<float>();

        return newNode;
    }
};

template <>
struct std::formatter<CapsuleCollider> : std::formatter<Node> {};
template <>
struct std::formatter<CapsuleCollider*> : std::formatter<Node*> {};
#endif