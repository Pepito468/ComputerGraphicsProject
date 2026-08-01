#ifndef ENGINE_BEHAVIOUR_HPP
#define ENGINE_BEHAVIOUR_HPP

#include "Node.hpp"
#include "modules/Colliders.hpp"

/// A node with custom logic
class Behaviour : public Node
{
    public:
        /// Called every frame by the engine
        virtual void update() {}
        /// Called when
        virtual void onCollision(Collider* other) {}
        virtual void onTriggerEnter(Collider* other) {}
        virtual void onTriggerExit(Collider* other) {}
        virtual void onTriggerStay(Collider* other) {}
};
#endif
