#ifndef PHYSICS
#define PHYSICS

#include "Collider.hpp"
#include "Relations.hpp"
#include <glm/glm.hpp>
#include <set>

struct BoundSet
{
    Collider* collider;
    const SphereBounds* sphereBounds;
    const AABBExtents* aabbExtents;
    const PointSet* pointSet;

    explicit BoundSet(Collider* coll)
    {
        collider = coll;
        sphereBounds = coll->getSphereBounds();
        aabbExtents = coll->getAABBExtents();
        pointSet = coll->getPointSet();
    }

    ~BoundSet()
    {
        delete sphereBounds;
        delete aabbExtents;
        delete pointSet;
    }
};

class Physics
{
    /// Root node of the current scene
    static Node* sceneRoot;
    /// All collider nodes in the scene
    static std::set<Collider*> colliders;
    /// Bounds of all colliders marked STATIC
    static std::set<BoundSet*> staticBounds;
    /// All collider nodes marked MOBILE
    static std::set<Collider*> dynamicColliders;

    static void findCollidersRecursive(const Node* node)
    {
        if (Collider* c = (Collider*)node)
        {
            colliders.insert(c);
            if (c->movementStatus == STATIC)
                staticBounds.insert(new BoundSet(c));
            else
                dynamicColliders.insert(c);
        }

        for (const Node* child : node->children)
        {
            findCollidersRecursive(child);
        }
    }

    static bool hasCollision(const BoundSet* a, const BoundSet* b)
    {
        //Sphere bounds check
        if (!SphereBounds::overlap(*a->sphereBounds, *b->sphereBounds)) return false;

        //AABB check
        if (!AABBExtents::overlaps(*a->aabbExtents, *b->aabbExtents)) return false;

        //Points check
        for (const glm::vec3 p : *a->pointSet)
        {
            if (b->collider->inBounds(p)) return true;
        }
        for (const glm::vec3 p : *b->pointSet)
        {
            if (a->collider->inBounds(p)) return true;
        }

        return false;
    }

    static void collisionCallbacks(Collider* moving, Collider* receiver)
    {
        if (!receiver->isTrigger)
        {
            if (receiver->onCollision) receiver->onCollision(moving);
        }
        else
        {
            //Trigger volume
            if (!receiver->collidersInTrigger.contains(moving))
            {
                if (receiver->onTriggerEnter) receiver->onTriggerEnter(moving);
                receiver->collidersInTrigger.insert(moving);
            }
            else
            {
                if (receiver->onTriggerStay) receiver->onTriggerStay(moving);
            }
        }
    }

    static void processCollisions(Collider* coll, std::set<BoundSet*>& collisions, const std::set<BoundSet*>& sceneBounds)
    {
        const bool hasHardCollision = std::ranges::any_of(collisions,
        [](const BoundSet* b) { return !b->collider->isTrigger; });
        if (!coll->isTrigger && hasHardCollision)
        {
            //Reverse collider's movements
            coll->globalMatrix = coll->previousMatrix;
            coll->updateGlobalTransformPropertiesFromGlobalMatrix();
            Node3D::updateLocalTransformFromGlobal(coll, coll->fatherMatrix);
        }

        for (const BoundSet* b : collisions)
        {
            Collider* bColl = b->collider;
            collisionCallbacks(coll, bColl);
        }

        //Check for trigger exits
        for (BoundSet* b : sceneBounds)
        {
            Collider* bColl = b->collider;
            if (bColl->isTrigger && bColl->collidersInTrigger.contains(coll) && !collisions.contains(b))
            {
                bColl->collidersInTrigger.erase(coll);
                if (bColl->onTriggerExit) bColl->onTriggerExit(coll);
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
            for (const BoundSet* b : staticBounds) delete b;
            staticBounds.clear();

            findCollidersRecursive(root);
        }

        /// Checks if there have been any collisions since the last call of this method
        static void checkCollisions()
        {
            std::set<BoundSet*> sceneBounds;
            std::ranges::transform(dynamicColliders, std::inserter(sceneBounds, sceneBounds.begin()),
            [](Collider* c) { return new BoundSet(c); });
            std::ranges::copy(staticBounds, std::inserter(sceneBounds, sceneBounds.begin()));

            std::set<Collider*> toCheck; //Cols that have moved since last check
            std::ranges::copy_if(dynamicColliders, std::inserter(toCheck, toCheck.begin()),
            [](const Collider* c) { return c->isActive && c->movementStatus == MOBILE_HAS_MOVED; });


            for (Collider* coll : toCheck)
            {
                BoundSet bounds = BoundSet(coll);

                //Find collisions
                std::set<BoundSet*> collisions;
                std::ranges::copy_if(sceneBounds, std::inserter(collisions, collisions.begin()),
                [bounds](const BoundSet* b)
                {
                    //Identity
                    if (b->collider->UUID == bounds.collider->UUID) return false;
                    return hasCollision(&bounds, b);
                });

                //Process collisions
                processCollisions(coll, collisions, sceneBounds);

                //Reset flag values
                coll->movementStatus = MOBILE_UNMOVED;
                coll->previousMatrix = coll->globalMatrix;
            }
        }

        struct RayCastHit
        {
            glm::vec3 point;
            float distance;
            Collider* collider;
        };
        /**
         * Shoots a ray from an origin point in a given direction and checks if it hits a collider.
         * @param origin The origin point of the ray.
         * @param direction The direction of the ray.
         * @param hit The struct in which to store the hit result.
         * @param maxDistance The maximum length of the ray.
         * @param step The distance between each point to check.
         * @return True if the ray hit something.
         */
        static bool raycast(const glm::vec3 origin, glm::vec3 direction, RayCastHit* hit = nullptr, const float maxDistance = 100.0f, const float step = 10 * EPSILON)
        {
            const int stepNum = maxDistance / step;
            direction = glm::normalize(direction);

            for (int i = 0; i < stepNum; i++)
            {
                const glm::vec3 p = origin + direction * (step * i);
                for (Collider* coll : colliders)
                {
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
};
#endif