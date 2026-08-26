#ifndef PHYSICS
#define PHYSICS

#include "Collider.hpp"
#include "Relations.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <set>

struct Bounds
{
    Collider* collider;
    SphereBounds sphereBounds;
    AABBExtents aabbExtents;
    PointSet pointSet;

    explicit Bounds(Collider* coll)
    {
        collider = coll;
        sphereBounds = coll->getSphereBounds();
        aabbExtents = coll->getAABBExtents();
        pointSet = coll->getPointSet();
    }

    ~Bounds() = default;

    bool operator<(const Bounds& other) const { return collider->UUID < other.collider->UUID; }
};

class Physics
{
    /// Root node of the current scene
    inline static Node* sceneRoot;
    /// All collider nodes in the scene
    inline static std::vector<Collider*> colliders = {};
    /// Bounds of all colliders marked STATIC
    inline static std::vector<Bounds> staticBounds;
    /// All collider nodes marked MOBILE
    inline static std::vector<Collider*> dynamicColliders;

    static void findCollidersRecursive(Node* node)
    {
        //log(std::format("Checking {}", node->name));
        if (Collider* c = dynamic_cast<Collider*>(node))
        {
            //log(std::format("Is collider A/S {}/{}", c->isActive, (int)c->movementStatus));
            colliders.push_back(c);
            if (c->movementStatus == STATIC)
                staticBounds.push_back(Bounds(c));
            else
                dynamicColliders.push_back(c);
        }

        for (Node* child : node->children)
        {
            findCollidersRecursive(child);
        }
    }

    static bool hasCollision(const Bounds& a, const Bounds& b)
    {
        //Sphere bounds check
        if (!SphereBounds::overlap(a.sphereBounds, b.sphereBounds)) return false;

        //AABB check
        if (!AABBExtents::overlaps(a.aabbExtents, b.aabbExtents)) return false;

        //Points check
        for (const glm::vec3 p : a.pointSet)
        {
            if (b.collider->inBounds(p)) return true;
        }
        for (const glm::vec3 p : b.pointSet)
        {
            if (a.collider->inBounds(p)) return true;
        }

        return false;
    }

    static void collisionCallbacks(Collider* moving, Collider* receiver)
    {
        //log(std::format("Receiver {} has: {}", receiver, receiver->collidersInTrigger));
        if (!receiver->isTrigger)
        {
            receiver->onCollision(moving);
            moving->onCollision(receiver);
        }
        else if (!receiver->collidersInTrigger.contains(moving))
        {
            receiver->onTriggerEnter(moving);
            receiver->collidersInTrigger.insert(moving);
        }
        else
            receiver->onTriggerStay(moving);
    }

    static void processCollisions(Collider* coll, const std::set<Bounds>& collisions, const std::set<Bounds>& sceneBounds)
    {
        const bool hasHardCollision = std::ranges::any_of(collisions,
        [](const Bounds& b) { return !b.collider->isTrigger; });
        if (!coll->isTrigger && hasHardCollision)
        {
            //Reverse collider's movements
            coll->setGlobalMatrix(coll->previousMatrix);
        }

        for (const Bounds b : collisions)
        {
            //log(std::format("{} collided with {}", coll->name, b.collider->name));
            Collider* bColl = b.collider;
            collisionCallbacks(coll, bColl);
        }

        //Check for trigger exits
        for (Bounds b : sceneBounds)
        {
            Collider* bColl = b.collider;
            //log(std::format("Exit check {}-{}: {} {} {}", bColl->name, coll->name,  bColl->isTrigger, bColl->collidersInTrigger.contains(coll), !collisions.contains(b)));
            if (bColl->isTrigger && bColl->collidersInTrigger.contains(coll) && !collisions.contains(b))
            {
                bColl->collidersInTrigger.erase(coll);
                bColl->onTriggerExit(coll);
            }
        }
    }

public:
    /// Loads a new scene into the physics system and finds all colliders within it.
    static void loadScene(Node* root)
    {
        sceneRoot = root;

        colliders.clear();
        dynamicColliders.clear();
        staticBounds.clear();

        findCollidersRecursive(root);
    }

    /// Checks if there have been any collisions since the last call of this method
    static void checkCollisions()
    {
        std::set<Bounds> sceneBounds;
        std::ranges::transform(dynamicColliders, std::inserter(sceneBounds, sceneBounds.begin()),
        [](Collider* c) { return Bounds(c); });
        std::ranges::copy(staticBounds, std::inserter(sceneBounds, sceneBounds.begin()));

        std::set<Collider*> toCheck; //Cols that have moved since last check
        std::ranges::copy_if(dynamicColliders, std::inserter(toCheck, toCheck.begin()),
        [](Collider* c)
        {
            bool isInTrigger = false;
            for (const Collider* coll : colliders)
            {
                if (coll->isActive && coll->isTrigger && coll->collidersInTrigger.contains(c))
                {
                    isInTrigger = true;
                    break;
                }
            }

            return c->isActive && (c->movementStatus == MOBILE_HAS_MOVED || isInTrigger);
        });

        std::set<Collider*> checked;
        for (Collider* coll : toCheck)
        {
            Bounds bounds = Bounds(coll);

            //Find collisions
            std::set<Bounds> collisions;
            std::ranges::copy_if(sceneBounds, std::inserter(collisions, collisions.begin()),
            [bounds, checked](const Bounds& b)
            {
                if (checked.contains(b.collider)) return false; //Skip checked colliders
                if (b.collider->UUID == bounds.collider->UUID) return false; //Skip self
                if (!(bounds.collider->collidesWith & b.collider->layer)) return false; //Skip mismatched layers
                return hasCollision(bounds, b);
            });

            //Process collisions
            processCollisions(coll, collisions, sceneBounds);

            //Reset flag values
            coll->movementStatus = MOBILE_UNMOVED;
            coll->previousMatrix = coll->getGlobalMatrix();

            checked.insert(coll);
        }
    }

    struct RaycastHit
    {
        glm::vec3 point = VEC3_ZERO;
        float distance = 0.0f;
        Collider* collider = nullptr;
    };
    /**
     * Shoots a ray from an origin point in a given direction and checks if it hits a collider.
     * @param origin The origin point of the ray.
     * @param direction The direction of the ray.
     * @param hit The struct in which to store the hit result.
     * @param maxDistance The maximum length of the ray.
     * @param step The distance between each point to check.
     * @param layer Physics layer flags to check.
     * @return True if the ray hit something.
     */
    static bool raycast(const glm::vec3 origin, glm::vec3 direction, RaycastHit* hit = nullptr, const float maxDistance = 100.0f, const float step = 10 * EPSILON, const PhysicsLayer layer = ALL)
    {
        const int stepNum = maxDistance / step;
        direction = glm::normalize(direction);

        for (int i = 0; i < stepNum; i++)
        {
            const glm::vec3 p = origin + direction * (step * i);
            for (Collider* coll : colliders)
            {
                if (!(coll->layer & layer)) continue;
                if (!coll->isActive) continue;
                if (coll->inBounds(p))
                {
                    if (hit)
                    {
                        hit->point = p;
                        hit->distance = step * i;
                        hit->collider = coll;
                    }
                    return true;
                }
            }
        }
        return false;
    }
    struct RaycastOptions
    {
        RaycastHit* hit = nullptr;
        float maxDistance = 100.0f;
        float step = 10 * EPSILON;
        PhysicsLayer layer = ALL;
    };
    static bool raycast(const glm::vec3 origin, glm::vec3 direction, const RaycastOptions& opts)
    {
        return raycast(origin, direction, opts.hit, opts.maxDistance, opts.step, opts.layer);
    }
};
#endif
