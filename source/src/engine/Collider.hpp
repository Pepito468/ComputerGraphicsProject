#ifndef COLLIDER_HPP
#define COLLIDER_HPP

#include "glm/ext/vector_float3.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "Node3D.hpp"
#include "glm/gtx/vector_angle.hpp"

/// A node with a physics collider
class Collider: public Node3D
{
    public:
        /// If the collider allows objects to pass through it
        bool isTrigger;

        /// External function to call when another collider collides with this one
        void (*onCollision)(Collider*);
        /// External function to call when another collider enters this trigger collider
        void (*onTriggerEnter)(Collider*);
        /// External function to call when another collider exits this trigger collider
        void (*onTriggerExit)(Collider*);
        /// External function to call when another collider remains in this trigger collider
        void (*onTriggerStay)(Collider*);

        Collider(): isTrigger(false), onTriggerEnter(nullptr), onTriggerExit(nullptr), onTriggerStay(nullptr) {}

        /// Checks if a point is within the bounds of the collider.
        virtual bool inBounds(glm::vec3 point) = 0;

        /// Checks if a collider intersects with this one.
        /// // NOTE: I had to change it to pass by reference since cpp doesn't like abstract classes passed by value
        virtual bool intersects(const Collider& coll) = 0;
};

/// A collider shaped like a box
class BoxCollider: public Collider
{
    public:
        /// The length of the box along the X-axis
        float width;

        /// The length of the box along the Y-axis
        float height;

        /// The length of the box along the Z-axis
        float depth;

        BoxCollider(): width(1.0f), height(1.0f), depth(1.0f) {}

        bool inBounds(const glm::vec3 point) override
        {
            const glm::vec3 p = this->toLocalSpace(point);
            glm::vec3 position = this->getPosition();
            const float dX = std::abs(position.x - p.x);
            const float dY = std::abs(position.y - p.y);
            const float dZ = std::abs(position.z - p.z);

            return dX <= width / 2 && dY <= height / 2 && dZ <= depth / 2;
        }

        bool intersects(const Collider& coll) override
        {
            //TODO
            return false;
        }
};

/// A spherical collider
class SphereCollider: public Collider
{
    public:
        /// The radius of the sphere
        float radius;

        SphereCollider(): radius(1) {}

        bool inBounds(const glm::vec3 point) override
        {
            return glm::distance(point, this->getPosition()) <= radius;
        }

        bool intersects(const Collider& coll) override
        {
            return false; //TODO
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

        CapsuleCollider(): radius(1), height(1) {}

        bool inBounds(const glm::vec3 point) override
        {
            const glm::vec3 p = this->toLocalSpace(point);
            const float dY = p.y - this->getPosition().y;
            if (-height / 2 <= dY && dY <= height / 2)
            {
                //Cylindrical
                const float dX = std::abs(p.x - this->getPosition().x);
                const float dZ = std::abs(p.z - this->getPosition().z);
                return (dX * dX + dZ * dZ) <= radius * radius;
            }

            //Sphere caps
            const glm::vec3 capC = this->getPosition() + (glm::vec3(0.0f, 1.0f, 0.0f) * (height / 2) * glm::sign(dY));


            return glm::distance(p, capC) <= radius;
        }

        bool intersects(const Collider& coll) override
        {
            return false; //TODO
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

        ConeCollider(): radius(1.0f), aperture(glm::radians(45.0f)) {}

        bool inBounds(const glm::vec3 point) override
        {
            if (!(glm::distance(point, this->getPosition()) <= radius)) return false;

            const glm::vec3 p = glm::normalize(point - this->getPosition());
            const float angle = glm::acos(glm::dot(this->getPosition(), p));
            return angle <= aperture;
        }

        bool intersects(const Collider& coll) override
        {
            return false; //TODO
        }
};
#endif
