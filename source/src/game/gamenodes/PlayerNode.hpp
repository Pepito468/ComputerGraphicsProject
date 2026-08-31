#ifndef ENGINE_PLAYERNODE_HPP
#define ENGINE_PLAYERNODE_HPP

#include "BulletNode.hpp"
#include "SphereCollider.hpp"
#include "Text2D.hpp"
#include "UpdateNode3D.hpp"
#include "Collider.hpp"
#include "Engine.hpp"
#include "AudioNode3D.hpp"
#include "glm/geometric.hpp"

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
    float walk_speed = 4.0f;

    AudioNode *quack = nullptr;

    ToonMaterial bulletMat = {glm::vec3(0.9f, 0.45f, 0.9f), {1.0f,1.0f,1.0f,100.0f}, 0.3f, 1.0f, 0.3f, 0.95f, 1.0f, 0.0f};

    void shoot()
    {
        //create bullet
        // SphereCollider* bulletColl = new SphereCollider();
        // bulletColl->name = "bullColl";
        // bulletColl->layer = NONE;
        // bulletColl->collidesWith = ALL;
        // BulletNode* bullet = new BulletNode();
        // bullet->name = "bullet";
        // Model3D *bulletMod = new Model3D("SuzanneUV.obj", {0, 0, 0}, {0, 0, 0}, {0.5f, 0.5f, 0.5f}, &bulletMat);
        // bulletMod->name = "bullMod";
        // bulletColl->adopt(bullet);
        // bulletColl->adopt(bulletMod);
        //
        // const glm::vec3 dir = -getZAxis();
        // const glm::vec3 v = glm::cross(VEC3_Z, dir);
        // const float angle = acos(glm::dot(VEC3_Z, dir));
        // const glm::mat4 rotMat = glm::rotate(angle, v);
        // bulletColl->setGlobalMatrix(rotMat);
        // bulletColl->setGlobalPosition(getGlobalPosition());
        //
        // Engine::instantiate(bulletColl);
        //
        // quack->playSound();

        BulletNode* bullet = new BulletNode();
        bullet->name = "bullet";
        Model3D *bulletMod = new Model3D("SuzanneUV.obj", {0, 0, 0}, {0, 0, 0}, {0.5f, 0.5f, 0.5f}, &bulletMat);
        bulletMod->name = "bullMod";
        bullet->adopt(bulletMod);

        const glm::vec3 dir = -getZAxis();
        const glm::vec3 v = glm::cross(VEC3_Z, dir);
        const float angle = acos(glm::dot(VEC3_Z, dir));
        const glm::mat4 rotMat = glm::rotate(angle, v);
        bullet->setGlobalMatrix(rotMat);
        bullet->setGlobalPosition(getGlobalPosition());

        Engine::instantiate(bullet);

        quack->playSound();
    }

public:

    AudioNode3D *music = nullptr;
    Text2D *inputtxt = nullptr;
    bool isMusicPlaying = false;

    void onEnter() override
    {
        // Set cursor mode
        // Engine::setCursorMode(GLFW_CURSOR_DISABLED);

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

        for (auto child : this->children)
            if (child->name == "quack")
                this->quack = dynamic_cast<AudioNode*>(child);
    }

    void onExit() override {
        printf("Player deleted!\n");
    }

    void update() override
    {
        // Check for escape
        if (Engine::isKeyBeingPressed(GLFW_KEY_C)) {
            Engine::MainEngine->requestEngineShutdown();
        }

        // Give cursor back if needed
        if (Engine::isKeyBeingPressed(GLFW_KEY_ESCAPE, true)) {
            if (Engine::isPauseMenuOpen()) {
                Engine::setCursorMode(GLFW_CURSOR_DISABLED);
            } else {
                Engine::setCursorMode(GLFW_CURSOR_NORMAL);
            }

            Engine::togglePauseMenu();
        }

        if (Engine::isKeyBeingPressed(GLFW_MOUSE_BUTTON_LEFT, true)) {
            if (Engine::isPauseMenuOpen()) {
                Engine::handleMenuClick();
            } // else {} if the left mouse button does something outside of menus
        }

        // Change scene
        if (Engine::isKeyBeingPressed(GLFW_KEY_K, true)) {
            Engine::requestSceneChange(Engine::getSceneFromNameMap("Scene2"));
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

        float speed = walk_speed;
        if (Engine::isKeyBeingPressed(GLFW_KEY_LEFT_SHIFT))
            speed *= 1.5;


        // update the camera position and direction with the inputs
        glm::vec3 delta = VEC3_ZERO;
        glm::vec3 normalizedXAxis = glm::normalize(glm::vec3(getXAxis().x, 0, getXAxis().z));
        glm::vec3 normalizedZAxis = glm::normalize(glm::vec3(getZAxis().x, 0, getZAxis().z));
        delta += normalizedXAxis * speed * Engine::getDeltaTime() * Engine::getInputTranslation().x;
        delta += normalizedZAxis * speed * Engine::getDeltaTime() * Engine::getInputTranslation().z;
        delta.y = vertSpeed * Engine::getDeltaTime();

        playerColl->globalTranslate(delta);

        /// When the pause menu is open, moving the mouse doesn't move the camera
        //TODO when the pause menu is open, "freeze" the game (or we can keep it running, like Dark Souls)
        if (!Engine::isPauseMenuOpen()) {
            const float xRot = -Engine::getInputRotation().x * Engine::getDeltaTime();
            if (X_ROT_MIN <= getGlobalRotation().x + xRot && getGlobalRotation().x + xRot <= X_ROT_MAX)
                globalRotateX(xRot);
            globalRotateY(-Engine::getInputRotation().y * Engine::getDeltaTime());
        }

        //Shoot
        if (Engine::isKeyBeingPressed(GLFW_KEY_F))
            shoot();

        // Start music
        if (Engine::isKeyBeingPressed(GLFW_KEY_M, true)) {
            if (!isMusicPlaying) {
                music->playSound();
                music->enableLooping();
                isMusicPlaying = true;
            } else {
                music->stopSound();
                music->disableLooping();
                isMusicPlaying = false;
            }
        }

        static float elapsedT = 1.0f;
        static float count = 0;
        elapsedT += Engine::getDeltaTime();
        count++;
        if (elapsedT >= 1.0f) {
            inputtxt->text = std::format("FPS: {}", count / elapsedT);
            elapsedT = 0.0f;
            count = 0;
            log(std::format("DATA: {} {} {} {}\n",
                        Engine::MainEngine->DPSZs.sampledImagesInPool,
                        Engine::MainEngine->DPSZs.samplersInPool,
                        Engine::MainEngine->DPSZs.setsInPool,
                        Engine::MainEngine->DPSZs.texturesInPool,
                        Engine::MainEngine->DPSZs.uniformBlocksInPool));
        }
    }
};
#endif
