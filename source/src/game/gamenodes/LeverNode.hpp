#ifndef ENGINE_LEVER_NODE
#define ENGINE_LEVER_NODE
#include "Engine.hpp"
#include "InteractableNode.hpp"
#include "AudioNode3D.hpp"
#include "SphereCollider.hpp"

// Initial and final rotations for the lever handle and bridge objects
#define LEVER_UP_ROT glm::radians(50.0f)
#define LEVER_DOWN_ROT glm::radians(-50.0f)
#define BRIDGE_UP_ROT glm::radians(-80.0f)
#define BRIDGE_DOWN_ROT 0.0f
#define PULL_TIME 2.0f
#define L_ROT_SPEED (LEVER_DOWN_ROT - LEVER_UP_ROT) / PULL_TIME
#define B_ROT_SPEED (BRIDGE_DOWN_ROT - BRIDGE_UP_ROT) / PULL_TIME

class LeverNode : public InteractableNode
{
    bool hasBeenPulled = false;
    Node3D* handle = nullptr;
    Node3D* bridge = nullptr;
    Collider* bridgeWall = nullptr;

    Collider* coll = nullptr;
    float timer = 0.0f;

    bool firstTimePull = true;
    AudioNode3D *leverSound = nullptr;
    AudioNode3D *bridgeSound = nullptr;

public:
    void interact() override
    {
        hasBeenPulled = true;
    }

    void update() override
    {
        if (!hasBeenPulled) return;

        if (!handle || !bridge ||!bridgeWall)
        {
            warning("LeverNode has NULL pointers", true);
            return;
        }

        timer += Engine::getDeltaTime();
        if (timer >= PULL_TIME)
        {
            bridgeWall->isActive = false;
            return;
        }

        if (firstTimePull && leverSound && bridgeSound) {
            leverSound->playSound();
            bridgeSound->playSound();
            firstTimePull = false;
        }

        handle->localRotateX(L_ROT_SPEED * Engine::getDeltaTime());
        bridge->localRotateX(B_ROT_SPEED * Engine::getDeltaTime());
    }

    static Node3D* makeStandardLever(Node3D* bridge, Collider* bridgeWall, AudioNode3D *leverSound, AudioNode3D *bridgeSound)
    {
        LeverNode* lever = new LeverNode();
        LambertTexMaterial* mat = new LambertTexMaterial(VEC3_ONE, {1, 1, 1, 100}, "wood-metal.png");
        Model3D* lever_b = new Model3D("Lever_Base.gltf", VEC3_ZERO, VEC3_ZERO, VEC3_ONE, mat);
        lever->adopt(lever_b);
        Model3D* lever_h = new Model3D("Lever_Handle.gltf", VEC3_ZERO, VEC3_ZERO, VEC3_ONE, mat);
        lever->adopt(lever_h);
        lever->handle = lever_h;
        Collider* leverColl = new SphereCollider();
        leverColl->name = "LeverCollider";
        leverColl->layer = INTERACTABLE;
        leverColl->collidesWith = NONE;
        leverColl->movementStatus = STATIC;
        lever->coll = leverColl;
        lever->adopt(leverColl);

        lever->globalScaleAll(VEC3_ONE * 2.0f);
        lever_h->localRotateX(LEVER_UP_ROT);

        lever->bridge = bridge;
        bridge->localRotateX(BRIDGE_UP_ROT);
        lever->bridgeWall = bridgeWall;

        lever->leverSound = leverSound;
        lever->bridgeSound = bridgeSound;

        return lever;
    }
};
#endif
