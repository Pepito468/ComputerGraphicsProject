#ifndef COLLIDER_HPP
#define COLLIDER_HPP

#define GLM_ENABLE_EXPERIMENTAL
#include <set>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "Node3D.hpp"

struct Vec3Compare {
    bool operator()(const glm::vec3& a, const glm::vec3& b) const {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    }
};
typedef std::set<glm::vec3, Vec3Compare> PointSet;
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
    public:
        /// If the collider allows objects to pass through it
        bool isTrigger = false;

        /// External function to call when another collider collides with this one
        void (*onCollision)(Collider&) = nullptr;
        /// External function to call when another collider enters this trigger collider
        void (*onTriggerEnter)(Collider&) = nullptr;
        /// External function to call when another collider exits this trigger collider
        void (*onTriggerExit)(Collider&) = nullptr;
        /// External function to call when another collider remains in this trigger collider
        void (*onTriggerStay)(Collider&) = nullptr;

        Collider() : Node3D() {}
        Collider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Node3D(position, rotation, scale) {}
        explicit Collider(const bool isTrigger) : Node3D()
        {
            this->isTrigger = isTrigger;
        }
        Collider(const bool isTrigger, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Node3D(position, rotation, scale)
        {
            this->isTrigger = isTrigger;
        }

        /// @return True if the point is within the bounds of the collider.
        virtual bool inBounds(glm::vec3 point) const = 0;

        /// @return A set containing points within the collider.
        virtual PointSet* getPointSet() const = 0;

        /// @return The sphere bounds containing the collider.
        virtual SphereBounds* getSphereBounds() const = 0;

        /// @return The bounding box containing the collider.
        virtual AABBExtents* getAABBExtents() const = 0;
};

/// A collider shaped like a box
class BoxCollider: public Collider
{
    PointSet* getCorners() const
    {
        PointSet* res = new PointSet();

        res->insert(toGlobalSpace(glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f)));
        res->insert(toGlobalSpace(glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f)));
        res->insert(toGlobalSpace(glm::vec3(width / 2.0f, -height / 2.0f, depth / 2.0f)));
        res->insert(toGlobalSpace(glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f)));
        res->insert(toGlobalSpace(glm::vec3(-width / 2.0f, height / 2.0f, depth / 2.0f)));
        res->insert(toGlobalSpace(glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f)));
        res->insert(toGlobalSpace(glm::vec3(-width / 2.0f, -height / 2.0f, depth / 2.0f)));
        res->insert(toGlobalSpace(glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f)));

        return res;
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
            const float dX = std::abs(position.x - p.x);
            const float dY = std::abs(position.y - p.y);
            const float dZ = std::abs(position.z - p.z);
            log(std::format("Delta: [{}, {}, {}]", dX, dY, dZ));
            return dX <= width / 2.0f && dY <= height / 2.0f && dZ <= depth / 2.0f;
        }

        PointSet* getPointSet() const override
        {
            PointSet* res = new PointSet();

            //Add the corners
            const PointSet* corners = getCorners();
            res->insert(corners->cbegin(), corners->cend());
            delete corners;

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
                        res->insert(toGlobalSpace(p));
                    }
                }
            }

            return res;
        }

        SphereBounds* getSphereBounds() const override
        {
            const glm::vec3 corner = toGlobalSpace(glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f));

            return new SphereBounds(position, glm::distance(position, corner));
        }

        AABBExtents* getAABBExtents() const override
        {
            const PointSet* corners = getCorners();
            AABBExtents* res = new AABBExtents(*corners);
            delete corners;
            return res;
        }
};

/// A spherical collider
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
            return glm::length(p) <= radius;
        }

        PointSet* getPointSet() const override
        {
            PointSet* res = new PointSet();

            //Add sphere center
            res->insert(position);

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
                    res->insert(toGlobalSpace(pointer));
                    pointer = glm::rotate(MAT4_I, pointsAngle, loopAxis) * glm::vec4(pointer, 1.0f);
                }
                loopAxis = glm::rotate(MAT4_I, loopAngle, VEC3_Y) * glm::vec4(loopAxis, 1.0f);
            }

            return res;
        }

        SphereBounds* getSphereBounds() const override
        {
            const float maxScale = std::max(std::max(scale.x, scale.y), scale.z);
            return new SphereBounds(position, radius * maxScale);
        }

        AABBExtents* getAABBExtents() const override
        {
            PointSet* pts = new PointSet();
            pts->insert(toGlobalSpace(VEC3_X * radius));
            pts->insert(toGlobalSpace(-VEC3_X * radius));
            pts->insert(toGlobalSpace(VEC3_Y * radius));
            pts->insert(toGlobalSpace(-VEC3_Y * radius));
            pts->insert(toGlobalSpace(VEC3_Z * radius));
            pts->insert(toGlobalSpace(-VEC3_Z * radius));
            pts->insert(position + VEC3_X * radius * glm::dot(scale, VEC3_X));
            pts->insert(position + VEC3_Y * radius * glm::dot(scale, VEC3_Y));
            pts->insert(position + VEC3_Z * radius * glm::dot(scale, VEC3_Z));
            pts->insert(position - VEC3_X * radius * glm::dot(scale, VEC3_X));
            pts->insert(position - VEC3_Y * radius * glm::dot(scale, VEC3_Y));
            pts->insert(position - VEC3_Z * radius * glm::dot(scale, VEC3_Z));

            AABBExtents* res = new AABBExtents(*pts);
            delete pts;
            return res;
        }
};

/// A collider shaped like a cylinder with two hemispheres at the ends
class CapsuleCollider: public Collider
{
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

        PointSet* getPointSet() const override
        {
            return {};
        }

        SphereBounds* getSphereBounds() const override
        {
            return nullptr;
        }

        AABBExtents* getAABBExtents() const override
        {
            return nullptr;
        }
};

/// A collider shaped like a spherical cone
class ConeCollider : public Collider
{
    public:
        /// The radius of the cone along the Z-axis
        float radius = 1.0f;

        /// The angle of the sector, in radians
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
            if (!(glm::distance(p, position) <= radius)) return false;

            const glm::vec3 toP = glm::normalize(p - position);
            const float angle = glm::acos(glm::dot(position, toP));
            return angle <= aperture;
        }

        PointSet* getPointSet() const override
        {
            return nullptr;
        }

        SphereBounds* getSphereBounds() const override
        {
            return nullptr;
        }

        AABBExtents* getAABBExtents() const override
        {
            return nullptr;
        }
};
#endif
