#ifndef ENGINE_BULLETNODE_HPP
#define ENGINE_BULLETNODE_HPP

#include "UpdateNode3D.hpp"
#include "Collider.hpp"
#include "Engine.hpp"
#include "GLMDebug.hpp"

#define SPEED 3.0f
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
                if (other->name != "PlayerCollider")
                    hide();
            };
        }
        else
        {
            error("Bullet node's parent is not a collider!");
        }
    }

    void update() override
    {
        timer += Engine::getDeltaTime();
        if (timer >= LIFETIME)
        {
            hide();
            return;
        }

        bulletColl->globalTranslate(bulletColl->getZAxis() * SPEED * Engine::getDeltaTime());
    }

    void shoot(const glm::vec3 pos, const glm::vec3 dir)
    {
        log("Shooting Node " + name);

        const glm::vec3 z = bulletColl->getZAxis();
        const glm::vec3 v = glm::cross(z, dir);
        const float angle = acos(glm::dot(z, dir) / (glm::length(z) * glm::length(dir)));
        const glm::mat4 rotMat = glm::rotate(angle, v);
        bulletColl->setGlobalMatrix(rotMat * bulletColl->getGlobalMatrix());
        bulletColl->setGlobalPosition(pos);
        bulletColl->isActive = true;
        isActive = true;
        timer = 0.0f;
    }

    void hide()
    {
        log("Hiding Node " + name);

        isActive = false;
        bulletColl->isActive = false;
        bulletColl->setGlobalPosition({0, -10, 0});
    }
};
#endif