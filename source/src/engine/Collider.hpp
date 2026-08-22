#ifndef ENGINE_Collider_HPP
#define ENGINE_Collider_HPP

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <set>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include "Node3D.hpp"
#include "Relations.hpp"

struct Vec3Compare {
    bool operator()(const glm::vec3& a, const glm::vec3& b) const {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    }
};
typedef std::set<glm::vec3, Vec3Compare> PointSet;
#define POINT_PARAMETER 12

/// The extents of the sphere bounds containing a Collider.
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

/// The extents of the axis-aligned bounding box containing a Collider.
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
    /// The Collider is not meant to move after being loaded into the scene
    STATIC,
    /// The Collider can move but hasn't since last it was checked
    MOBILE_UNMOVED,
    /// The Collider has moved since last it was checked
    MOBILE_HAS_MOVED
};
/// A node with a physics Collider
class Collider: public Node3D
{
    public:
        /// If the Collider is to be used in physics checks
        bool isActive = true;
        /// The Collider's movement status
        MovementStatus movementStatus = MOBILE_UNMOVED;
        /// The Collider's global matrix at the start of the previous frame
        glm::mat4 previousMatrix = MAT4_I;
        /// If the Collider allows objects to pass through it
        bool isTrigger = false;
        /// Colliders that are within this trigger's bounds
        std::set<Collider*> CollidersInTrigger = std::set<Collider*>();

        /// External function to call when another Collider collides with this one
        void (*onCollision)(Collider*) = nullptr;
        /// External function to call when another Collider enters this trigger Collider
        void (*onTriggerEnter)(Collider*) = nullptr;
        /// External function to call when another Collider exits this trigger Collider
        void (*onTriggerExit)(Collider*) = nullptr;
        /// External function to call when another Collider remains in this trigger Collider
        void (*onTriggerStay)(Collider*) = nullptr;

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

        /// @return True if the point is within the bounds of the Collider.
        virtual bool inBounds(glm::vec3 point) const = 0;

        /// @return A set containing points within the Collider.
        virtual PointSet getPointSet() const = 0;

        /// @return The sphere bounds containing the Collider.
        virtual SphereBounds getSphereBounds() const = 0;

        /// @return The bounding box containing the Collider.
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
};

/// A Collider shaped like a box
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
        float width = 1.0f;

        /// The length of the box along the Y-axis
        float height = 1.0f;

        /// The length of the box along the Z-axis
        float depth = 1.0f;

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
};

/// A spherical Collider
class SphereCollider: public Collider
{
    public:
        /// The radius of the sphere
        float radius = 1.0f;

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
            res.insert(toGlobalSpace(VEC3_Y * radius));
            res.insert(toGlobalSpace(-VEC3_Y * radius));

            //Add points along the surface
            const float cos30Rad = radius * std::cos(M_PI / 6.0f);
            const float sin30Rad = radius * std::sin(M_PI / 6.0f);
            const float cos45Rad = radius * std::cos(M_PI / 4.0f);
            glm::vec3 pointers[] = {
                glm::vec3(radius, 0, 0),
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

/// A Collider shaped like a cylinder with two hemispheres at the ends
class CapsuleCollider: public Collider
{
    float halfHeight() const {return height / 2.0f;}

    public:
        /// The radius of the capsule
        float radius = 1.0f;
        /// The length of the capsule's cylinder along the Y-axis
        float height = 1.0f;

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
                res.insert(toGlobalSpace({radius, y, 0}));
                res.insert(toGlobalSpace({-radius, y, 0}));
                res.insert(toGlobalSpace({0, y, radius}));
                res.insert(toGlobalSpace({0, y, -radius}));
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
};

/// A Collider shaped like a spherical cone
class ConeCollider : public Collider
{
    private:
        PointSet getOBBCorners() const
        {
            PointSet res = PointSet();

            const float endRad = radius * std::sin(aperture);
            res.insert(toGlobalSpace({endRad, endRad, 0}));
            res.insert(toGlobalSpace({endRad, -endRad, 0}));
            res.insert(toGlobalSpace({-endRad, endRad, 0}));
            res.insert(toGlobalSpace({-endRad, -endRad, 0}));
            res.insert(toGlobalSpace({endRad, endRad, radius}));
            res.insert(toGlobalSpace({endRad, -endRad, radius}));
            res.insert(toGlobalSpace({-endRad, endRad, radius}));
            res.insert(toGlobalSpace({-endRad, -endRad, radius}));
            return res;
        }

    public:
        /// The radius of the cone along the Z-axis
        float radius = 1.0f;

        /// The angle between the Z-axis of the cone and its extremes, in radians [0, PI / 2]
        float aperture = M_PI / 6;

        ConeCollider() : Collider() {}
        ConeCollider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(position, rotation, scale) {}
        ConeCollider(const bool isTrigger, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(isTrigger, position, rotation, scale) {}
        ConeCollider(const float radius, const float aperture) : Collider()
        {
            this->radius = radius;
            this->aperture = aperture;
        }

        bool inBounds(const glm::vec3 point) const override
        {
            const glm::vec3 p = toLocalSpace(point);
            if (!epsilonLessThanEqual(glm::length(p), radius)) return false;

            const float angle = glm::acos(glm::dot(VEC3_Z, glm::normalize(p)));
            return epsilonLessThanEqual(angle, aperture);
        }

        PointSet getPointSet() const override
        {
            PointSet res = PointSet();

            //Add origin and cap peak
            res.insert(getGlobalPosition());
            res.insert(toGlobalSpace({0, 0, radius}));

            glm::vec3 sidePointer = glm::rotate(MAT4_I, aperture, VEC3_X) * glm::vec4(VEC3_Z, 1);
            const float delta = radius / POINT_PARAMETER;
            constexpr float loopAngle = glm::radians(360.0f / POINT_PARAMETER);
            glm::vec3 capPointers[] = {
                glm::rotate(MAT4_I, aperture / 4, VEC3_X) * glm::vec4(VEC3_Z, 1),
                glm::rotate(MAT4_I, aperture / 2, VEC3_X) * glm::vec4(VEC3_Z, 1),
                glm::rotate(MAT4_I, aperture * 3 / 4, VEC3_X) * glm::vec4(VEC3_Z, 1)
            };

            for (int i = 0; i < POINT_PARAMETER; i++)
            {
                //Add points along sides
                for (int j = 1; j < POINT_PARAMETER; j++)
                {
                    res.insert(toGlobalSpace(sidePointer * (delta * j)));
                }
                sidePointer = glm::rotate(MAT4_I, loopAngle, VEC3_Z) * glm::vec4(sidePointer, 1);

                //Add points on cap
                for (int j = 0; j < std::size(capPointers); j++)
                {
                    res.insert(toGlobalSpace(capPointers[j]));
                    capPointers[j] = glm::rotate(MAT4_I, loopAngle, VEC3_Z) * glm::vec4(capPointers[j], 1);
                }
            }

            return res;
        }

        SphereBounds getSphereBounds() const override
        {
            const glm::vec3 center = toGlobalSpace({0, 0, radius / 2.0f});
            //Point on the circumference of the cone
            const glm::vec3 edgePoint = toGlobalSpace({radius * glm::sin(aperture), 0, radius * glm::cos(aperture)});

            return SphereBounds(center, glm::distance(center, edgePoint));
        }

        AABBExtents getAABBExtents() const override
        {
            return AABBExtents(getOBBCorners());
        }
};

/// std::format specialisations
template <>
struct std::formatter<Collider> : std::formatter<Node> {};
template <>
struct std::formatter<Collider*> : std::formatter<Node*> {};
template <>
struct std::formatter<BoxCollider> : std::formatter<Node> {};
template <>
struct std::formatter<BoxCollider*> : std::formatter<Node*> {};
template <>
struct std::formatter<SphereCollider> : std::formatter<Node> {};
template <>
struct std::formatter<SphereCollider*> : std::formatter<Node*> {};
template <>
struct std::formatter<CapsuleCollider> : std::formatter<Node> {};
template <>
struct std::formatter<CapsuleCollider*> : std::formatter<Node*> {};
template <>
struct std::formatter<ConeCollider> : std::formatter<Node> {};
template <>
struct std::formatter<ConeCollider*> : std::formatter<Node*> {};
#endif
