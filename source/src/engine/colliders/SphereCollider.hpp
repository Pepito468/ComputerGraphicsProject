#ifndef ENGINE_SPHERECOLLIDER_HPP
#define ENGINE_SPHERECOLLIDER_HPP

#include "Collider.hpp"

/// A spherical collider
class SphereCollider: public Collider
{
public:
    /// The radius of the sphere
    BoundFloat radius = PositiveFloat(1.0f);

    SphereCollider() : Collider() {}
    SphereCollider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(position, rotation, scale) {}
    SphereCollider(const bool isTrigger, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(isTrigger, position, rotation, scale) {}
    explicit SphereCollider(const float radius) : Collider()
    {
        this->radius = radius;
    }

    bool inBounds(const glm::vec3 point) const override
    {
        const glm::vec3 p = toLocalSpace(point);
        return epsilonLessThanEqual(glm::length(p), radius);
    }

    PointSet getPointSet() const override
    {
        PointSet res = PointSet();

        //Add sphere center
        res.insert(getGlobalPosition());

        //Add the poles
        res.insert(toGlobalSpace(VEC3_Y * (float)radius));
        res.insert(toGlobalSpace(-VEC3_Y * (float)radius));

        //Add points along the surface
        const float cos30Rad = radius * std::cos(M_PI / 6.0f);
        const float sin30Rad = radius * std::sin(M_PI / 6.0f);
        const float cos45Rad = radius * std::cos(M_PI / 4.0f);
        glm::vec3 pointers[] = {
            glm::vec3((float)radius, 0, 0),
            glm::vec3(cos30Rad, sin30Rad, 0), glm::vec3(cos45Rad, cos45Rad, 0), glm::vec3(sin30Rad, cos30Rad, 0),
            glm::vec3(cos30Rad, -sin30Rad, 0), glm::vec3(cos45Rad, -cos45Rad, 0), glm::vec3(sin30Rad, -cos30Rad, 0)
        };
        constexpr float loopAngle = glm::radians(360.0f / POINT_PARAMETER); // angle between loops

        for (int i = 0; i <= POINT_PARAMETER; i++)
        {
            for (int j = 0; j < std::size(pointers); j++)
            {
                res.insert(toGlobalSpace(pointers[j]));
                pointers[j] = glm::rotate(MAT4_I, loopAngle, VEC3_Y) * glm::vec4(pointers[j], 1.0f);
            }
        }

        return res;
    }

    SphereBounds getSphereBounds() const override
    {
        return SphereBounds(getGlobalPosition(), radius * maxComponent(getGlobalScale()));
    }

    AABBExtents getAABBExtents() const override
    {
        return getSphereExtents(radius, getGlobalMatrix());
    }

    static SphereCollider* fromJSON(const nlohmann::json& json, SphereCollider* node = nullptr) {
        SphereCollider *newNode = node ? node: new SphereCollider();

       newNode->collFromJSON(json);

        if (json.contains("radius")) newNode->radius = json.at("radius").get<float>();

        return newNode;
    }

    /**
     * Calculate the AABB extents of a sphere.
     * @param radius The radius of the sphere.
     * @param matrix The transform matrix applied to the sphere.
     * @return A pointer to the AABB extents.
     */
    static AABBExtents getSphereExtents(const float radius, const glm::mat4& matrix)
    {
        // Method for sphere extents by Tavian Barnes
        // https://tavianator.com/2014/ellipsoid_bounding_boxes.html
        AABBExtents res = AABBExtents();

        const double dX = radius * std::sqrt(glm::dot(glm::vec3(matrix[0][0], matrix[1][0], matrix[2][0]), glm::vec3(matrix[0][0], matrix[1][0], matrix[2][0])));
        const double dY = radius * std::sqrt(glm::dot(glm::vec3(matrix[0][1], matrix[1][1], matrix[2][1]), glm::vec3(matrix[0][1], matrix[1][1], matrix[2][1])));
        const double dZ = radius * std::sqrt(glm::dot(glm::vec3(matrix[0][2], matrix[1][2], matrix[2][2]), glm::vec3(matrix[0][2], matrix[1][2], matrix[2][2])));

        res.xMax = matrix[3][0] + dX;
        res.yMax = matrix[3][1] + dY;
        res.zMax = matrix[3][2] + dZ;
        res.xMin = matrix[3][0] - dX;
        res.yMin = matrix[3][1] - dY;
        res.zMin = matrix[3][2] - dZ;
        return res;
    }
};

template <>
struct std::formatter<SphereCollider> : std::formatter<Node> {};
template <>
struct std::formatter<SphereCollider*> : std::formatter<Node*> {};
#endif