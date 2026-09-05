#ifndef ENGINE_PLAYER_NODE_HPP
#define ENGINE_PLAYER_NODE_HPP

#include "BulletNode.hpp"
#include "Material.hpp"
#include "CapsuleCollider.hpp"
#include "Material.hpp"
#include "OrthoCamera.hpp"
#include "PerspectiveCamera.hpp"
#include "SphereCollider.hpp"
#include "UpdateNode3D.hpp"
#include "Collider.hpp"
#include "Engine.hpp"
#include "Physics.hpp"
#include "InteractableNode.hpp"

#define WALK_SPEED 10.0f
#define JUMP_FORCE 5.0f
#define GRAVITY 9.81f
#define X_ROT_MIN glm::radians(-70.0f)
#define X_ROT_MAX glm::radians(80.0f)
#define INTERACTION_DIST 7.0f

/**
 * Node representing the player
 * @note Requires the node's parent to be a Collider
 */
class PlayerNode : public UpdateNode3D
{
    Collider* playerColl = nullptr;
    float vertSpeed = 0.0f;
    bool isGrounded = false;

    InteractableNode* selectedInteraction = nullptr;

    AudioNode *quack = nullptr;

    ToonMaterial bulletMat = {glm::vec3(0.9f, 0.45f, 0.9f), {1.0f,1.0f,1.0f,100.0f}, 0.3f, 1.0f, 0.3f, 0.95f, 1.0f, 0.0f};
    RainbowMaterial rMat = {0.2, 1, 1, 0.3};

    SonarMaterial *sonarMat = nullptr;

    bool isCamPersp = true;
    OrthoCamera *mapCam = nullptr;
    PerspectiveCamera *pCam = nullptr;

    void shoot()
    {
        //create bullet
        SphereCollider* bulletColl = new SphereCollider();
        bulletColl->name = "bullColl";
        bulletColl->layer = BULLETS;
        bulletColl->collidesWith = ENVIRONMENT;
        BulletNode* bullet = new BulletNode();
        bullet->name = "bullet";
        Model3D *bulletMod = new Model3D("SuzanneUV.obj", {0, 0, 0}, {0, 0, 0}, {0.5f, 0.5f, 0.5f}, &rMat);
        bulletMod->name = "bullMod";
        bulletColl->adopt(bullet);
        bulletColl->adopt(bulletMod);

        const glm::vec3 dir = getLookingDirection();
        const glm::vec3 v = glm::cross(VEC3_Z, dir);
        const float angle = acos(glm::dot(VEC3_Z, dir));
        const glm::mat4 rotMat = glm::rotate(angle, v);
        bulletColl->setGlobalMatrix(rotMat);
        bulletColl->setGlobalPosition(getGlobalPosition());

        Engine::instantiate(bullet);

        if (quack)
            quack->playSound();
    }

    void select(InteractableNode* i)
    {
        if (selectedInteraction == i) return;

        log("Selected interactable: " + (i ? i->name : "NULL"));

        if (selectedInteraction)
            selectedInteraction->deselect();

        selectedInteraction = i;
        if (selectedInteraction)
            selectedInteraction->select();
    }

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

        for (auto child : this->children)
            if (child->name == "quack")
                this->quack = dynamic_cast<AudioNode*>(child);
    }

    void onExit() override {
        printf("Player deleted!\n");
    }

    void update() override
    {
        if (Engine::isPauseMenuOpen())
            return; // Game is paused, don't do anything

        //Testing for object placement TODO remove
        if (Engine::isKeyBeingPressed(GLFW_KEY_L, true))
        {
            Model3D* tree = new Model3D("Spooky Tree.gltf", getGlobalPosition() * glm::vec3(1, 0, 1), VEC3_ZERO, VEC3_ONE, &bulletMat);
            info(std::format("{}", tree->getGlobalPosition()));
            Engine::instantiate(tree);
        }

        //Check grounded
        isGrounded = Physics::raycast(playerColl->getGlobalPosition() - VEC3_Y * 1.5f, -VEC3_Y, {.maxDistance = 0.2f, .layer = ENVIRONMENT});
        if (isGrounded)
        {
            vertSpeed = 0.0f;
            if (Engine::isKeyBeingPressed(GLFW_KEY_SPACE))
                vertSpeed = JUMP_FORCE;
        }
        else
            vertSpeed -= GRAVITY * Engine::getDeltaTime();

        float speed = WALK_SPEED;
        if (Engine::isKeyBeingPressed(GLFW_KEY_LEFT_SHIFT))
            speed *= 1.5;

        // update the camera position and direction with the inputs
        glm::vec3 delta = VEC3_ZERO;
        glm::vec3 normalizedXAxis = glm::normalize(glm::vec3(getXAxis().x, 0, getXAxis().z));
        glm::vec3 normalizedZAxis = glm::normalize(glm::vec3(getZAxis().x, 0, getZAxis().z));
        delta += normalizedXAxis * speed * Engine::getDeltaTime() * Engine::getInputTranslation().x;
        delta += normalizedZAxis * speed * Engine::getDeltaTime() * Engine::getInputTranslation().z;
        delta.y = vertSpeed * Engine::getDeltaTime();

        //Clamp delta to avoid phasing through walls
#define MAX_MOVEMENT 0.5f
        delta.x = std::clamp(delta.x, -MAX_MOVEMENT, MAX_MOVEMENT);
        delta.y = std::clamp(delta.y, -MAX_MOVEMENT, MAX_MOVEMENT);
        delta.z = std::clamp(delta.z, -MAX_MOVEMENT, MAX_MOVEMENT);
        playerColl->globalTranslate(delta);

        const float xRot = -Engine::getInputRotation().x * Engine::getDeltaTime();
        if (X_ROT_MIN <= getGlobalRotation().x + xRot && getGlobalRotation().x + xRot <= X_ROT_MAX)
            globalRotateX(xRot);
        playerColl->globalRotateY(-Engine::getInputRotation().y * Engine::getDeltaTime());

        //Shoot
        if (Engine::isKeyBeingPressed(GLFW_KEY_F, true))
            shoot();

        //Check for interactable objects, assumes the collider is a child of the object
        Physics::RaycastHit hit;
        if (Physics::raycast(getGlobalPosition(), getLookingDirection(), &hit,  INTERACTION_DIST, INTERACTABLE))
        {
            if (InteractableNode* inter = dynamic_cast<InteractableNode*>(hit.collider->parent))
            {
                select(inter);
            }
            else
                warning(std::format("Collider {} is INTERACTABLE but has no interactable parent", hit.collider));
        }
        else
            select(nullptr);

        if (selectedInteraction && Engine::isKeyBeingPressed(GLFW_KEY_E, true))
            selectedInteraction->interact();

        if (sonarMat && Engine::isKeyBeingPressed(GLFW_MOUSE_BUTTON_LEFT, true)) {
            log("SONAR");
            sonarMat->trigger(this->getGlobalPosition(), Engine::getCurrentTime(), SonarMaterial::WHITE);
        } else if (sonarMat && Engine::isKeyBeingPressed(GLFW_KEY_T, true)) {
            log("SONAR");
            sonarMat->trigger(this->getGlobalPosition(), Engine::getCurrentTime(), SonarMaterial::RED);
        } else if (sonarMat && Engine::isKeyBeingPressed(GLFW_KEY_U, true)) {
            log("SONAR");
            sonarMat->trigger(this->getGlobalPosition(), Engine::getCurrentTime(), SonarMaterial::GREEN);
        } else if (sonarMat && Engine::isKeyBeingPressed(GLFW_KEY_Y, true)) {
            log("SONAR");
            sonarMat->trigger(this->getGlobalPosition(), Engine::getCurrentTime(), SonarMaterial::YELLOW);
        } else if (sonarMat && Engine::isKeyBeingPressed(GLFW_KEY_R, true)) {
            log("SONAR");
            sonarMat->trigger(this->getGlobalPosition(), Engine::getCurrentTime(), SonarMaterial::BLUE);
        }

        if (Engine::isKeyBeingPressed(GLFW_KEY_1)) {
            Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("Forest")));
        } else if (Engine::isKeyBeingPressed(GLFW_KEY_2)) {
            Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("Unit")));
        } else if (Engine::isKeyBeingPressed(GLFW_KEY_3)) {
            Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("Dark")));
        } else if (Engine::isKeyBeingPressed(GLFW_KEY_9)) {
            Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("Scene1")));
        } else if (Engine::isKeyBeingPressed(GLFW_KEY_0)) {
            Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("Scene2")));
        }

        if (this->mapCam && Engine::isKeyBeingPressed(GLFW_KEY_M, true)) {
            if (isCamPersp)
                Engine::setMainCamera(this->mapCam);
            else
                Engine::setMainCamera(this->pCam);
            isCamPersp = !isCamPersp;
        }
        if (this->mapCam)
            mapCam->setGlobalRotation({-std::numbers::pi/4, -5*std::numbers::pi/4, 0.0f});
    }

    /// Creates the standard node tree for the player.
    static Node3D* makeStandardPlayer(OrthoCamera *extraCamera = nullptr)
    {
        CapsuleCollider* rootCollider = new CapsuleCollider();
        rootCollider->name = "PlayerCollider";
        rootCollider->layer = PLAYER;
        rootCollider->collidesWith = ENVIRONMENT;

        PlayerNode* controls = new PlayerNode();
        controls->name = "Player";
        rootCollider->adopt(controls);
        controls->sonarMat = std::any_cast<SonarMaterial*>(Engine::getGlobalVariable("SonarMaterialReference"));
        controls->localTranslate({0, 2.75f, 0});

        controls->mapCam = extraCamera;

        PerspectiveCamera *camera = new PerspectiveCamera(0.1f, 400, glm::radians(90.0f), 4.0f/3.0f, true);
        camera->name = "PerspectiveCamera";
        controls->pCam = camera;
        controls->adopt(camera);

        LambertTexMaterial* mat = new LambertTexMaterial(VEC3_ONE, {1, 1, 1, 100}, "mage.png");
        Model3D* model = new Model3D("Mage.gltf", VEC3_ZERO, {0, M_PI, 0}, VEC3_ONE, mat, true);
        rootCollider->adopt(model);

        AudioNode *quack = new AudioNode("quack.mp3", 0.1f);
        quack->name = "quack";
        controls->adopt(quack);

        rootCollider->globalTranslate({0, 5, 0});
        rootCollider->localRotateY(M_PI);
        return rootCollider;
    }
};
#endif
