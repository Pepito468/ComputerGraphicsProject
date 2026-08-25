#ifndef ENGINE_COLLIDER_HPP
#define ENGINE_COLLIDER_HPP

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <set>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include "Node3D.hpp"
#include "Relations.hpp"
#include "Types.hpp"

struct Vec3Compare {
    bool operator()(const glm::vec3& a, const glm::vec3& b) const {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    }
};
typedef std::set<glm::vec3, Vec3Compare> PointSet;
#define POINT_PARAMETER 12

/// The extents of the sphere bounds containing a collider.
struct SphereBounds
{
    glm::vec3 center;
    float radius;

    SphereBounds()
    {
        this->center = glm::vec3(0.0f, 0.0f, 0.0f);
        this->radius = 0.0f;
    }
    SphereBounds(const glm::vec3& center, const float radius)
    {
        this->center = center;
        this->radius = radius;
    }

    /// @return True if there is overlap between the two bounding spheres.
    static bool overlap(const SphereBounds& a, const SphereBounds& b)
    {
        return epsilonLessThanEqual(glm::distance(a.center, b.center), a.radius + b.radius);
    }
};

/// The extents of the axis-aligned bounding box containing a collider.
struct AABBExtents
{
    float xMin, xMax;
    float yMin, yMax;
    float zMin, zMax;

    AABBExtents()
    {
        xMax = yMax = zMax = -INFINITY;
        xMin = yMin = zMin = INFINITY;
    }
    explicit AABBExtents(const PointSet& points)
    {
        xMax = yMax = zMax = -INFINITY;
        xMin = yMin = zMin = INFINITY;

        for (const glm::vec3 p : points)
        {
            if (p.x < xMin) xMin = p.x;
            if (p.y < yMin) yMin = p.y;
            if (p.z < zMin) zMin = p.z;
            if (p.x > xMax) xMax = p.x;
            if (p.y > yMax) yMax = p.y;
            if (p.z > zMax) zMax = p.z;
        }
    }

    /// @return True if there is overlap between the two bounding boxes.
    static bool overlaps(const AABBExtents& a, const AABBExtents& b)
    {
        return (a.xMin <= b.xMax) && (b.xMax >= b.xMin)
            && (a.yMin <= b.yMax) && (b.yMax >= b.yMin)
            && (a.zMin <= b.zMax) && (b.zMax >= b.zMin);
    }
};

enum MovementStatus
{
    /// The collider is not meant to move after being loaded into the scene
    STATIC,
    /// The collider can move but hasn't since last it was checked
    MOBILE_UNMOVED,
    /// The collider has moved since last it was checked
    MOBILE_HAS_MOVED
};
NLOHMANN_JSON_SERIALIZE_ENUM(MovementStatus, {
    {STATIC, "static"},
    {MOBILE_UNMOVED, "dynamic"}
})

enum PhysicsLayer
{
    NONE = 0,
    ENVIRONMENT = 1,
    PLAYER = 2,
    ALL = ENVIRONMENT | PLAYER
};
NLOHMANN_JSON_SERIALIZE_ENUM(PhysicsLayer, {
    {ENVIRONMENT, "env"},
    {PLAYER, "player"},
    {ALL, "all"}
})

/// A node with a physics collider
class Collider: public Node3D
{
public:
    /// If the collider is to be used in physics checks
    bool isActive = true;
    /// The collider's movement status
    MovementStatus movementStatus = MOBILE_UNMOVED;
    /// The collider's global matrix at the start of the previous frame
    glm::mat4 previousMatrix = MAT4_I;
    /// If the collider allows objects to pass through it
    bool isTrigger = false;
    /// Colliders that are within this trigger's bounds
    std::set<Collider*> collidersInTrigger = std::set<Collider*>();
    /// Physics layer the collider is included in
    PhysicsLayer layer = ALL;

    typedef Function<void(Collider*)> CollisionCallback;
    /// External function to call when another collider collides with this one
    CollisionCallback onCollision;
    /// External function to call when another collider enters this trigger collider
    CollisionCallback onTriggerEnter;
    /// External function to call when another collider exits this trigger collider
    CollisionCallback onTriggerExit;
    /// External function to call when another collider remains in this trigger collider
    CollisionCallback onTriggerStay;

    Collider() : Node3D() {}
    Collider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Node3D(position, rotation, scale)
    {
        previousMatrix = getGlobalMatrix();
    }
    explicit Collider(const bool isTrigger) : Node3D()
    {
        this->isTrigger = isTrigger;
        previousMatrix = getGlobalMatrix();
    }
    Collider(const bool isTrigger, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Node3D(position, rotation, scale)
    {
        this->isTrigger = isTrigger;
        previousMatrix = getGlobalMatrix();
    }

    /// @return True if the point is within the bounds of the collider.
    virtual bool inBounds(glm::vec3 point) const = 0;

    /// @return A set containing points within the collider.
    virtual PointSet getPointSet() const = 0;

    /// @return The sphere bounds containing the collider.
    virtual SphereBounds getSphereBounds() const = 0;

    /// @return The bounding box containing the collider.
    virtual AABBExtents getAABBExtents() const = 0;

    void commitGlobalUpdate() override
    {
        if (movementStatus == STATIC) warning(std::format("Collider {} has moved, even though it was marked static!", name));

        movementStatus = MOBILE_HAS_MOVED;
        Node3D::commitGlobalUpdate();
    }

    void commitLocalUpdate() override
    {
        if (movementStatus == STATIC) warning(std::format("Collider {} has moved, even though it was marked static!", name));

        movementStatus = MOBILE_HAS_MOVED;
        Node3D::commitLocalUpdate();
    }

protected:

    void collFromJSON(const nlohmann::json& json)
    {
        Node3D::fromJSON(json, this);

        if (json.contains("isTrigger")) isTrigger = json.at("isTrigger").get<bool>();
        if (json.contains("movement")) movementStatus = json.at("movement").get<MovementStatus>();
        if (json.contains("layer")) layer = json.at("layer").get<PhysicsLayer>();
    }
};

/// std::format specialisations
template <>
struct std::formatter<Collider> : std::formatter<Node> {};
template <>
struct std::formatter<Collider*> : std::formatter<Node*> {};
#endif
