#ifndef ENGINE_PLAYERNODE_HPP
#define ENGINE_PLAYERNODE_HPP

#include "UpdateNode3D.hpp"
#include "Collider.hpp"
#include "Camera.hpp"
#include "Engine.hpp"
#include "GLMDebug.hpp"

#define WALK_SPEED 4.0f
#define JUMP_FORCE 5.0f
#define GRAVITY 9.81f
#define X_ROT_MIN glm::radians(-70.0f)
#define X_ROT_MAX glm::radians(80.0f)

/**
 * Node representing the player
 * @note Requires the node's parent to be a Collider
 */
class PlayerNode : public UpdateNode3D
{
    Collider* playerColl = nullptr;
    float vertSpeed = 0.0f;
    bool isGrounded = false;

public:

    void onEnter() override
    {
        if (Collider* c = dynamic_cast<Collider*>(this->parent))
        {
            playerColl = c;
            playerColl->onCollision = [](const Collider* other)
            {
                log("Player collided with " + other->name);
            };
        }
        else
        {
            error("Player node's parent is not a collider!");
        }
    }

    void update() override
    {
        // Check for escape
        if(Engine::isKeyBeingPressed(GLFW_KEY_ESCAPE)) {
            Engine::MainEngine->shutdown();
        }

        //Check grounded
        isGrounded = Physics::raycast(playerColl->getGlobalPosition() - VEC3_Y * 1.35f, -VEC3_Y, {.maxDistance = 1.0f, .layer = ENVIRONMENT});
        if (isGrounded)
        {
            vertSpeed = 0.0f;
            if (Engine::isKeyBeingPressed(GLFW_KEY_SPACE))
                vertSpeed = JUMP_FORCE;
        }
        else
            vertSpeed -= GRAVITY * Engine::getDeltaTime();

        // update the camera position and direction with the inputs
        glm::vec3 delta = VEC3_ZERO;
        delta += getXAxis() * WALK_SPEED * Engine::getDeltaTime() * Engine::getInputTranslation().x;
        delta += getZAxis() * WALK_SPEED * Engine::getDeltaTime() * Engine::getInputTranslation().z;
        delta.y = vertSpeed * Engine::getDeltaTime();

        playerColl->globalTranslate(delta);

        const float xRot = -Engine::getInputRotation().x * Engine::getDeltaTime();
        if (X_ROT_MIN <= getGlobalRotation().x + xRot && getGlobalRotation().x + xRot <= X_ROT_MAX)
            globalRotateX(xRot);
        globalRotateY(-Engine::getInputRotation().y * Engine::getDeltaTime());
    }
};
#endif