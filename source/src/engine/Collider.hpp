#ifndef COLLIDER_HPP
#define COLLIDER_HPP

#define GLM_ENABLE_EXPERIMENTAL
#include <list>
#include <set>

#include "glm/ext/vector_float3.hpp"
#include "glm/gtx/vector_angle.hpp"

#include "Node3D.hpp"

#define POINT_COUNT 40

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
    bool overlaps(const SphereBounds& other) const
    {
        return glm::distance(this->center, other.center) <= this->radius + other.radius;
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
    explicit AABBExtents(const std::set<glm::vec3>& points)
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
    bool overlaps(const AABBExtents& other) const
    {
        return (xMin <= other.xMax) && (xMax >= other.xMin)
            && (yMin <= other.yMax) && (yMax >= other.yMin)
            && (zMin <= other.zMax) && (zMax >= other.zMin);
    }
};

/// A node with a physics collider
class Collider: public Node3D
{
    /*
    // Helper functions for collisions of different types
    virtual bool intersects_Box(const BoxCollider& coll) const = 0;
    virtual bool intersects_Sphere(const SphereCollider& coll) const = 0;
    virtual bool intersects_Capsule(const CapsuleCollider& coll) const = 0;
    virtual bool intersects_Cone(const ConeCollider& coll) const = 0;

    AABBExtents getAABBExtents() const = 0;
    */
    public:
        /// If the collider allows objects to pass through it
        bool isTrigger;

        /// External function to call when another collider collides with this one
        void (*onCollision)(Collider&);
        /// External function to call when another collider enters this trigger collider
        void (*onTriggerEnter)(Collider&);
        /// External function to call when another collider exits this trigger collider
        void (*onTriggerExit)(Collider&);
        /// External function to call when another collider remains in this trigger collider
        void (*onTriggerStay)(Collider&);

        Collider()
        {
            isTrigger = false;

            onCollision = nullptr;
            onTriggerEnter = nullptr;
            onTriggerExit = nullptr;
            onTriggerStay = nullptr;
        }
        Collider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Node3D(position, rotation, scale), Collider() {}

        /// @return True if the point is within the bounds of the collider.
        virtual bool inBounds(glm::vec3 point) const = 0;

        /// @return A set containing points within the collider.
        virtual std::set<glm::vec3> getPointSet() const = 0;

        /// @return The sphere bounds containing the collider.
        virtual SphereBounds getSphereBounds() const = 0;

        /// @return The bounding box containing the collider.
        virtual AABBExtents getAABBExtents() const = 0;

    /*
        /// Checks if a collider intersects with this one.
        /// NOTE: I had to change it to pass by reference since cpp doesn't like abstract classes passed by value
        bool intersects(Collider* coll) const
        {
            if (const BoxCollider* ptr = dynamic_cast<BoxCollider*>(coll); ptr != nullptr)
            {
                return this->intersects_Box(*ptr);
            }
            if (const SphereColliderCollider* ptr = dynamic_cast<SphereCollider*>(coll); ptr != nullptr)
            {
                return this->intersects_Sphere(*ptr);
            }
            if (const CapsuleCollider* ptr = dynamic_cast<CapsuleCollider*>(coll); ptr != nullptr)
            {
                return this->intersects_Capsule(*ptr);
            }
            if (const ConeCollider* ptr = dynamic_cast<ConeCollider*>(coll); ptr != nullptr)
            {
                return this->intersects_Cone(*ptr);
            }

            error(str::format("Collider::intersects not implemented for {}", typeid(&coll)));
            return false;
        }
        */
};

/// A collider shaped like a box
class BoxCollider: public Collider
{
    std::set<glm::vec3> getCorners() const
    {
        std::set<glm::vec3> res = {};

        res.insert(toGlobalSpace(glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f)));
        res.insert(toGlobalSpace(glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f)));
        res.insert(toGlobalSpace(glm::vec3(width / 2.0f, -height / 2.0f, depth / 2.0f)));
        res.insert(toGlobalSpace(glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f)));
        res.insert(toGlobalSpace(glm::vec3(-width / 2.0f, height / 2.0f, depth / 2.0f)));
        res.insert(toGlobalSpace(glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f)));
        res.insert(toGlobalSpace(glm::vec3(-width / 2.0f, -height / 2.0f, depth / 2.0f)));
        res.insert(toGlobalSpace(glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f)));

        return res;
    }

    public:
        /// The length of the box along the X-axis
        float width;

        /// The length of the box along the Y-axis
        float height;

        /// The length of the box along the Z-axis
        float depth;

        BoxCollider()
        {
            width = 1.0f;
            height = 1.0f;
            depth = 1.0f;
        }
        BoxCollider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(position, rotation, scale), BoxCollider() {}

        bool inBounds(const glm::vec3 point) const override
        {
            const glm::vec3 p = toLocalSpace(point);
            const float dX = std::abs(position.x - p.x);
            const float dY = std::abs(position.y - p.y);
            const float dZ = std::abs(position.z - p.z);

            return dX <= width / 2 && dY <= height / 2 && dZ <= depth / 2;
        }

        std::set<glm::vec3> getPointSet() const override
        {
            std::set<glm::vec3> res = {};

            //Add the corners
            std::set<glm::vec3> corners = getCorners();
            res.insert(corners.begin(), corners.end());

            //Add in between
            const double countRoot3 = cbrt(POINT_COUNT);
            const double dx = width / countRoot3;
            const double dy = height / countRoot3;
            const double dz = depth / countRoot3;
            const glm::vec3 bottomCorner = glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f);

            for (int i = 0; i < countRoot3; i++)
            {
                for (int j = 0; j < countRoot3; j++)
                {
                    for (int k = 0; k < countRoot3; k++)
                    {
                        const glm::vec3 p = bottomCorner + glm::vec3(dx * i, dy * j, dz * k);
                        res.insert(toGlobalSpace(p));
                    }
                }
            }

            return res;
        }

        SphereBounds getSphereBounds() const override
        {
            const glm::vec3 corner = toGlobalSpace(glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f));

            return SphereBounds(position, glm::distance(position, corner));
        }

        AABBExtents getAABBExtents() const override
        {
            return AABBExtents(getCorners());
        }
};

/// A spherical collider
class SphereCollider: public Collider
{
    public:
        /// The radius of the sphere
        float radius;

        SphereCollider()
        {
            radius = 1.0f;
        }
        SphereCollider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(position, rotation, scale), SphereCollider() {}

        bool inBounds(const glm::vec3 point) const override
        {
            const glm::vec3 p = toLocalSpace(point);
            return glm::length(p) <= radius;
        }

        std::set<glm::vec3> getPointSet() const override
        {
            const std::set<glm::vec3> res = {};

            //Add sphere center
            res.insert(position);

            constexpr int loopNum = 6;
            const int pointsPerLoop = ceil(static_cast<float>(POINT_COUNT) / loopNum);
            const float pointsAngle = glm::radians(360.0f / pointsPerLoop); // angle between points in the same loop
            constexpr float loopAngle = glm::radians(360.0f / loopNum); // angle between loops
            glm::vec3 pointer = VEC3_Y * radius;
            glm::vec3 loopAxis = VEC3_Z;

            for (int i = 0; i < loopNum; i++)
            {
                for (int j = 0; j < pointsPerLoop; j++)
                {
                    res.insert(toGlobalSpace(pointer));
                    pointer = glm::rotate(pointer, pointsAngle, loopAxis);
                }
                loopAxis = glm::rotate(loopAxis, loopAngle, VEC3_Y);
            }

            return res;
        }

        SphereBounds getSphereBounds() const override
        {
            return SphereBounds(position, radius * glm::max(scale.x, scale.y, scale.z));
        }

        AABBExtents getAABBExtents() const override
        {
            std::set<glm::vec3> pts = {};
            pts.insert(toGlobalSpace(VEC3_X * radius));
            pts.insert(toGlobalSpace(-VEC3_X * radius));
            pts.insert(toGlobalSpace(VEC3_Y * radius));
            pts.insert(toGlobalSpace(-VEC3_Y * radius));
            pts.insert(toGlobalSpace(VEC3_Z * radius));
            pts.insert(toGlobalSpace(-VEC3_Z * radius));
            pts.insert(position + VEC3_X * radius * glm::dot(scale, VEC3_X));
            pts.insert(position + VEC3_Y * radius * glm::dot(scale, VEC3_Y));
            pts.insert(position + VEC3_Z * radius * glm::dot(scale, VEC3_Z));
            pts.insert(position - VEC3_X * radius * glm::dot(scale, VEC3_X));
            pts.insert(position - VEC3_Y * radius * glm::dot(scale, VEC3_Y));
            pts.insert(position - VEC3_Z * radius * glm::dot(scale, VEC3_Z));
            return AABBExtents(pts);
        }
};

/// A collider shaped like a cylinder with two hemispheres at the ends
class CapsuleCollider: public Collider
{
    public:
        /// The radius of the capsule
        float radius;
        /// The length of the capsule's cylinder along the Y-axis
        float height;

        CapsuleCollider()
        {
            radius = 1.0f;
            height = 1.0f;
        }
        CapsuleCollider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(position, rotation, scale), CapsuleCollider() {}

        bool inBounds(const glm::vec3 point) const override
        {
            const glm::vec3 p = toLocalSpace(point);
            const float dY = p.y - position.y;
            if (-height / 2 <= dY && dY <= height / 2)
            {
                //Cylindrical
                const float dX = std::abs(p.x - position.x);
                const float dZ = std::abs(p.z - position.z);
                return (dX * dX + dZ * dZ) <= radius * radius;
            }

            //Sphere caps
            const glm::vec3 capC = position + (VEC3_Y * (height / 2) * glm::sign(dY));
            return glm::distance(p, capC) <= radius;
        }

        std::set<glm::vec3> getPointSet() const override
        {

        }

        SphereBounds getSphereBounds() const override
        {

        }

        AABBExtents getAABBExtents() const override
        {

        }
};

/// A collider shaped like a spherical cone
class ConeCollider : public Collider
{
    public:
        /// The radius of the cone along the Z-axis
        float radius;

        /// The angle of the sector, in radians
        float aperture;

        ConeCollider()
        {
            radius = 1.0f;
            aperture = glm::radians(30.0f);
        }
        ConeCollider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(position, rotation, scale), ConeCollider() {}

        bool inBounds(const glm::vec3 point) const override
        {
            const glm::vec3 p = toLocalSpace(point);
            if (!(glm::distance(p, position) <= radius)) return false;

            const glm::vec3 toP = glm::normalize(p - position);
            const float angle = glm::acos(glm::dot(position, toP));
            return angle <= aperture;
        }

        std::set<glm::vec3> getPointSet() const override
        {

        }

        SphereBounds getSphereBounds() const override
        {

        }

        AABBExtents getAABBExtents() const override
        {

        }
};
#endif
