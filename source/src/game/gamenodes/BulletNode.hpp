#ifndef ENGINE_BULLETNODE_HPP
#define ENGINE_BULLETNODE_HPP

#include "UpdateNode3D.hpp"
#include "Collider.hpp"
#include "Engine.hpp"
#include "GLMDebug.hpp"

#define SPEED 4.0f
#define LIFETIME 5.0f

/**
 * Node representing a bullet that moves forward along its Z-axis
 * @note Requires the node's parent to be a Collider
 */
class BulletNode : public UpdateNode3D
{
    Collider* bulletColl = nullptr;
    float timer = 0.0f;

public:
    void onEnter() override
    {
        if (Collider* c = dynamic_cast<Collider*>(this->parent))
        {
            bulletColl = c;
            bulletColl->onCollision = [this](const Collider* other)
            {
                log("Bullet collided with " + other->name);
                Engine::requestNodeDeletion(bulletColl, true);
            };
        }
        else
        {
            error("Bullet node's parent is not a collider!");
        }
    }

    void update() override
    {
        //log(std::format("Bullet flying for {}. At {}/{}", timer, bulletColl->getGlobalPosition(), getGlobalPosition()));
        timer += Engine::getDeltaTime();
        if (timer >= LIFETIME)
        {
            Engine::requestNodeDeletion(bulletColl, true);
            return;
        }

        bulletColl->globalTranslate(bulletColl->getZAxis() * SPEED * Engine::getDeltaTime());
    }
};
#endif
