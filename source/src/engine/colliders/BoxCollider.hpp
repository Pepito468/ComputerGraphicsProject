#ifndef ENGINE_BOXCOLLIDER_HPP
#define ENGINE_BOXCOLLIDER_HPP

#include "Collider.hpp"

/// A collider shaped like a box
class BoxCollider: public Collider
{
    float halfWidth() const {return width / 2.0f;}
    float halfHeight() const {return height / 2.0f;}
    float halfDepth() const {return depth / 2.0f;}

    void getCorners(PointSet* set) const
    {
        set->insert(toGlobalSpace(glm::vec3(halfWidth(), halfHeight(), halfDepth())));
        set->insert(toGlobalSpace(glm::vec3(halfWidth(), halfHeight(), -halfDepth())));
        set->insert(toGlobalSpace(glm::vec3(halfWidth(), -halfHeight(), halfDepth())));
        set->insert(toGlobalSpace(glm::vec3(halfWidth(), -halfHeight(), -halfDepth())));
        set->insert(toGlobalSpace(glm::vec3(-halfWidth(), halfHeight(), halfDepth())));
        set->insert(toGlobalSpace(glm::vec3(-halfWidth(), halfHeight(), -halfDepth())));
        set->insert(toGlobalSpace(glm::vec3(-halfWidth(), -halfHeight(), halfDepth())));
        set->insert(toGlobalSpace(glm::vec3(-halfWidth(), -halfHeight(), -halfDepth())));
    }

public:
    /// The length of the box along the X-axis
    BoundFloat width = PositiveFloat(1.0f);

    /// The length of the box along the Y-axis
    BoundFloat height = PositiveFloat(1.0f);

    /// The length of the box along the Z-axis
    BoundFloat depth = PositiveFloat(1.0f);

    BoxCollider() : Collider() {}
    BoxCollider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(position, rotation, scale) {}
    BoxCollider(const bool isTrigger, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(isTrigger, position, rotation, scale) {}
    BoxCollider(const float width, const float height, const float depth) : Collider()
    {
        this->width = width;
        this->height = height;
        this->depth = depth;
    }

    bool inBounds(const glm::vec3 point) const override
    {
        const glm::vec3 p = toLocalSpace(point);
        const glm::vec3 dist = glm::abs(p);
        const glm::vec3 limit = {halfWidth(), halfHeight(), halfDepth()};
        return epsilonLessThanEqual(dist, limit);
    }

    PointSet getPointSet() const override
    {
        PointSet res = PointSet();

        //Add the box's center
        res.insert(getGlobalPosition());

        //Add points along the faces
        const double dx = width  / POINT_PARAMETER;
        const double dy = height / POINT_PARAMETER;
        const double dz = depth  / POINT_PARAMETER;
        for (int x = 0; x < POINT_PARAMETER; x++)
        {
            for (int y = 0; y < POINT_PARAMETER; ++y)
            {
                res.insert(toGlobalSpace({-halfWidth() + dx * x, -halfHeight() + dy * y, -halfDepth()}));
                res.insert(toGlobalSpace({-halfWidth() + dx * x, -halfHeight() + dy * y, halfDepth()}));
            }
        }
        for (int x = 0; x < POINT_PARAMETER; x++)
        {
            for (int z = 0; z < POINT_PARAMETER; z++)
            {
                res.insert(toGlobalSpace({-halfWidth() + dx * x, -halfHeight(), -halfDepth() + dz * z}));
                res.insert(toGlobalSpace({-halfWidth() + dx * x, halfHeight(), -halfDepth() + dz * z}));
            }
        }
        for (int y = 0; y < POINT_PARAMETER; ++y)
        {
            for (int z = 0; z < POINT_PARAMETER; z++)
            {
                res.insert(toGlobalSpace({-halfWidth(), -halfHeight() + dy * y, -halfDepth() + dz * z}));
                res.insert(toGlobalSpace({halfWidth(), -halfHeight() + dy * y, -halfDepth() + dz * z}));
            }
        }

        return res;
    }

    SphereBounds getSphereBounds() const override
    {
        const glm::vec3 corner = toGlobalSpace(glm::vec3(halfWidth(), halfHeight(), halfDepth()));
        return SphereBounds(getGlobalPosition(), glm::distance(getGlobalPosition(), corner));
    }

    AABBExtents getAABBExtents() const override
    {
        PointSet corners = PointSet();
        getCorners(&corners);
        return AABBExtents(corners);
    }

    static BoxCollider* fromJSON(const nlohmann::json& json, BoxCollider* node = nullptr) {
        BoxCollider *newNode = node ? node: new BoxCollider();

        newNode->collFromJSON(json);

        if (json.contains("width")) newNode->width = json.at("width").get<float>();
        if (json.contains("height")) newNode->height = json.at("height").get<float>();
        if (json.contains("depth")) newNode->depth = json.at("depth").get<float>();

        return newNode;
    }
};

template <>
struct std::formatter<BoxCollider> : std::formatter<Node> {};
template <>
struct std::formatter<BoxCollider*> : std::formatter<Node*> {};
#endif