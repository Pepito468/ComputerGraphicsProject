#include <any>
#include <iostream>
#include <numbers>
#include <ostream>

#include "Engine.hpp"

#include "Material.hpp"
#include "Node3D.hpp"
#include "PerspectiveCamera.hpp"
#include "OrthoCamera.hpp"

#include "Model3D.hpp"

#include "PointLight.hpp"
#include "DirectionalLight.hpp"
#include "AmbientLight.hpp"
#include "ColliderLib.hpp"

#include "Text2D.hpp"
#include "audio/AudioNode.hpp"
#include "audio/AudioNode3D.hpp"

#include "common.h"
#include "Particles.hpp"
#include "gamenodes/AudioController.hpp"
#include "gamenodes/CustomCameraUpdate.hpp"
#include "gamenodes/FPSTextUpdater.hpp"
#include "gamenodes/LeverNode.hpp"
#include "gamenodes/LightningNode.hpp"
#include "gamenodes/MovingPlanetUpdate.hpp"
#include "gamenodes/MovingPlanetChildUpdate.hpp"
#include "gamenodes/PlayerNode.hpp"
#include "gamenodes/SpinningMageUpdate.hpp"
#include "glm/ext/vector_float3.hpp"

#define TEX_MAT(texName) new LambertTexMaterial(VEC3_ONE, {1, 1, 1, 50}, texName)

float randNorm() {return (float)std::rand() / RAND_MAX;}

LambertMaterial mat3 = {glm::vec3(1.0f, 0.0f, 0.0f), {1.0f,1.0f,1.0f,100.0f}};
ToonMaterial mat2 = {glm::vec3(0.9f, 0.45f, 0.9f), {1.0f,1.0f,1.0f,100.0f}, 0.3f, 1.0f, 0.3f, 0.95f, 1.0f, 0.0f};
CookTorranceMaterial mat1 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "rock.png"};
//LambertTexMaterial mat2 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "VChecker.png"};
WaterMaterial mat4 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f},0.5f, "water.png"};
FireMaterial flame1 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "smallFlame.png"};
FireMaterial flame2 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "bigFlame.png"};
FireMaterial flame3 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "fire.png"};
MagicCirleMaterial magicMat = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "magicCircle.png"};

CookTorranceAnimMaterial animMat = {glm::vec3(1.0f, 0.0f, 0.0f), {1.0f,1.0f,1.0f,100.0f}, "rock.png"};

RainbowMaterial rMat = {0.2, 1, 1, 0.3};
SonarMaterial sMat = {20, 10, 75};

void freeNodeTree(Node *node) {
    for (Node *child : node->children)
        freeNodeTree(child);
    delete node;
}

LambertTexMaterial cubeMat = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "cube.png"};
LambertMaterial blankMat = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}};
LambertTexMaterial* mageMat = TEX_MAT("mage.png");
LambertMaterial treeMat = {{0.095f, 0.009f, 0.0f}, {1.0f,1.0f,1.0f,100.0f}};
WindMaterial* grassMat = new WindMaterial({-0.6f, 1}, 1.5f, 3, VEC3_ONE, {1, 1, 1, 100}, "grass.png");
LambertTexMaterial* pathMat = TEX_MAT("path.png");
LambertTexMaterial* lowGrassMat = TEX_MAT("low_grass.png");
LambertTexMaterial* wood_metalMat = TEX_MAT("wood-metal.png");
CookTorranceMaterial* rockMat = new CookTorranceMaterial({1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 100.0f},"rock.png");
WaterMaterial waterMat = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, 0.5f, "water.png"};
LambertMaterial rainMat = {glm::vec3(0.0f, 0.0f, .9f), {1.0f,1.0f,1.0f,100.0f}};
OutlineMaterial outlineMat = OutlineMaterial({1, 0, 1}, 4.0f);
LambertMaterial metalMat = {{0.153, 0.212, 0.322}, {0.153, 0.212, 0.322, 1}};

Node* createScene1() {

    Node *root = new Node();
    root->name = "root";

    Node3D* player = PlayerNode::makeStandardPlayer();
    root->adopt(player);

    //CustomCameraUpdate *cameraContainer = new CustomCameraUpdate();
    //cameraContainer->name = "CameraContainer";
    //PerspectiveCamera *camera1 = new PerspectiveCamera(0.01, 100, glm::radians(90.0f), 4.0f/3.0f);
    //camera1->name = "PerspectiveCamera";
    OrthoCamera *camera2 = new OrthoCamera(-4, 4, -2, 2, 0.01, 200);
    camera2->name = "OrthoCamera";
    player->adopt(camera2);
    //cameraContainer->adopt(camera1);
    //cameraContainer->adopt(camera2);

    //engine.setMainCamera(camera1);
    //
    // Text
    root->adopt(new FPSTextUpdater());

    // Models
    Node *models = new Node();
    models->name = "ModelContainer";
    root->adopt(models);

    Model3D *statue = new Model3D("Statue.gltf", {1, -0.5, 0}, {0, 0, 0}, {4, 4, 4}, &mat1);
    statue->name = "Statue";
    models->adopt(statue);

    Model3D *statueR = new Model3D("Statue.gltf", {-1, -0.5, 0}, {0, 0, 0}, {4, 4, 4}, &rMat);
    statueR->name = "Statue";
    models->adopt(statueR);

    // Sonar plane
    Model3D *sonar = new Model3D("Water.gltf", {-3, 3, -10}, {std::numbers::pi/2, 0, 0}, {10, 10, 10}, &sMat);
    sonar->name = "Sonar";
    models->adopt(sonar);
    Model3D *statues = new Model3D("Statue.gltf", {-5, 0, -7}, {0, 0, 0}, {5, 5, 5}, &sMat);
    statues->name = "Statue";
    models->adopt(statues);

    // Sonar statue


    Model3D *plane = new Model3D("Water.gltf", {0, 0.1, 0}, {0, 0, 0}, {10, 10, 10}, &mat1);
    BoxCollider* planeColl = new BoxCollider(2.0f, 0.05f, 2.0f);
    planeColl->name = "PlaneHB";
    planeColl->movementStatus = STATIC;
    planeColl->layer = ENVIRONMENT;
    plane->name = "Plane";
    plane->adopt(planeColl);
    models->adopt(plane);

    AnimatedModel3D *man = new AnimatedModel3D(
                                    {"assets/models/man/uomo.gltf",
                                                "assets/models/man/running.gltf",
                                                "assets/models/man/idle.gltf",
                                                "assets/models/man/pointing.gltf",
                                                "assets/models/man/waving.gltf"},
                                                {{0,32,0.0f,0}, {0,16,0.0f,1}, {0,263,0.0f,2}, {0,83,0.0f,3}, {0,16,0.0f,4}}
                                                ,"Armature|mixamo.com|Layer0", "Ch01_Body",
                                                {2, 0, 0}, {0, 0, 0}, {3, 3, 3},
                                                &animMat);
    models->adopt(man);

    Model3D *magicCircle = new Model3D("Water.gltf", {0, 0.5, 0}, {0, 0, 0}, {2, 2, 2}, &magicMat);
    models->adopt(magicCircle);

    Model3D *bigFire = new Model3D("Water.gltf", {0, 4, 0}, {glm::radians(90.0f), 0, glm::radians(90.0f)}, glm::vec3(1.0), &flame2);
    models->adopt(bigFire);

    Model3D *bigFire2 = new Model3D("Water.gltf", {0, 4, -2}, {glm::radians(90.0f), 0, glm::radians(90.0f)}, glm::vec3(1.0), &flame3);
    models->adopt(bigFire2);

    Model3D *smallFire = new Model3D("Water.gltf", {0, 4, 2}, {glm::radians(90.0f), 0, glm::radians(90.0f)}, glm::vec3(0.3), &flame1);
    models->adopt(smallFire);

    for (int i = -5; i <= 5; i++) {
        Model3D *m = new Model3D("Statue.gltf", {-1, -0.25, i}, {0, 0, 0}, {1, 1, 1}, &mat1);
        m->name = std::format("Statue-{}", i);
        models->adopt(m);
   }

    Model3D *suzanne = new Model3D("SuzanneUV.obj", {4, 0, 1}, {0, glm::radians(-90.0f), 0}, {1, 1, 1}, &mat2);
    suzanne->name = "Suzanne";
    models->adopt(suzanne);

    // Lights
    AmbientLight *ambientLight = new AmbientLight({0.08f, 0.14f, 0.20f},{0.035f, 0.04f, 0.045f}, {0.0f, 1.0f, 0.0f});
    ambientLight->name = "AmbientLight";
    root->adopt(ambientLight);

    DirectionalLight *directionalLight = new DirectionalLight(0.5,glm::vec3(1.0f, 0.95f, 0.8f),glm::normalize(glm::vec3(0.8f, 0.25f, 0.4f)));
    directionalLight->name = "DirectionalLight";
    root->adopt(directionalLight);

    /*
    PointLight *redLight = new PointLight({-10, 4, 0}, 0.1, {1, 0, 0}, 5, 2);
    redLight->name = "RedLight";
    root->adopt(redLight);

    PointLight *yellowLight = new PointLight({0, 4, 10}, 0.1, {1, 1, 0}, 5, 2);
    yellowLight->name = "YellowLight";
    root->adopt(yellowLight);

    PointLight *blueLight = new PointLight({0, 4, -10}, 0.1, {0, 0, 1}, 5, 2);
    blueLight->name = "BlueLight";
    root->adopt(blueLight);
    */

    // Planet
    MovingPlanetUpdate *planet = new MovingPlanetUpdate();
    planet->globalTranslate({0, 2, 0});
    planet->name = "Planet";
    root->adopt(planet);
    // Moving child
    PointLight *planetLight = new PointLight({0, 0, 0}, 0.075, {0.9, 0.5, 0}, 4, 2);
    planet->adopt(planetLight);
    // Moving statue
    Model3D *planetStatue = new Model3D("Statue.gltf", {0, 0, 0}, {0, 0, 0}, {1, 1, 1}, &mat1);
    planetStatue->name = "PlanetStatue";
    planet->adopt(planetStatue);
    // Planet child
    MovingPlanetChildUpdate *movingChild = new MovingPlanetChildUpdate();
    planet->adopt(movingChild);
    // Rotates around the planet
    Model3D *orbitingStatue = new Model3D("Statue.gltf", {0, 0, 0}, {0, 0, 0}, {1, 1, 1}, &mat1);
    orbitingStatue->localTranslate({1, 0, 0});
    movingChild->adopt(orbitingStatue);
    AudioController *movingAudio = new AudioController(new AudioNode3D("rocketJumpWaltz.mp3", 1.0f, 50.0f, 1.0f, INVERSE, 0.1f));
    movingAudio->name = "RocketJumWaltz";
    planet->adopt(movingAudio);

    return root;
}

Node* createScene2() {

    Node *root = new Node();
    root->name = "root";
    // CapsuleCollider* playerCollider = new CapsuleCollider();
    // playerCollider->name = "PlayerCollider";
    // playerCollider->layer = PLAYER;
    // root->adopt(playerCollider);
    // PlayerNode* player = new PlayerNode();
    // player->name = "Player";
    // playerCollider->adopt(player);
    // PerspectiveCamera *camera = new PerspectiveCamera(0.01, 100, glm::radians(90.0f), 4.0f/3.0f, true);
    // camera->name = "PerspectiveCamera";
    // player->adopt(camera);
    // playerCollider->globalTranslate({-2, 5, 0});
    // player->localTranslate({0, 1.25f, 0});

    CustomCameraUpdate *cameraContainer = new CustomCameraUpdate();
    // Position over the plane
    cameraContainer->globalTranslate({-2, 2, 0});
    cameraContainer->globalRotateY(glm::radians(-90.0f));
    cameraContainer->name = "CameraContainer";
    PerspectiveCamera *camera1 = new PerspectiveCamera(0.01, 100, glm::radians(90.0f), 4.0f/3.0f);
    camera1->name = "PerspectiveCamera";
    OrthoCamera *camera2 = new OrthoCamera(-4, 4, -2, 2, 0.01, 200);
    camera2->name = "OrthoCamera";
    root->adopt(cameraContainer);
    cameraContainer->adopt(camera1);
    cameraContainer->adopt(camera2);

    //engine.setMainCamera(camera1);

    // Models

    Node *models = new Node();
    models->name = "ModelContainer";
    root->adopt(models);

    Model3D *statue = new Model3D("Statue.gltf", {1, -0.5, 0}, {0, 0, 0}, {4, 4, 4}, &mat1);
    statue->name = "Statue";
    models->adopt(statue);

    Model3D *plane = new Model3D("Water.gltf", {0, 0.1, 0}, {0, 0, 0}, {10, 10, 10}, &mat4);
    BoxCollider* planeColl = new BoxCollider(2.0f, 0.01f, 2.0f);
    planeColl->name = "PlaneHB";
    planeColl->movementStatus = STATIC;
    planeColl->layer = ENVIRONMENT;
    plane->name = "Plane";
    plane->adopt(planeColl);
    models->adopt(plane);

    for (int i = -5; i <= 5; i++) {
        Model3D *m = new Model3D("Statue.gltf", {-1, 1, i}, {0, 0, 0}, {1, 1, 1}, &mat1);
        m->name = std::format("Statue-{}", i);
        models->adopt(m);
   }

    Model3D *suzanne = new Model3D("SuzanneUV.obj", {4, 2, 1}, {0, glm::radians(-90.0f), 0}, {1, 1, 1}, &mat2);
    suzanne->name = "Suzanne";
    models->adopt(suzanne);

    // Lights
    AmbientLight *ambientLight = new AmbientLight({0.08f, 0.14f, 0.20f},{0.035f, 0.04f, 0.045f}, {0.0f, 1.0f, 0.0f});
    ambientLight->name = "AmbientLight";
    root->adopt(ambientLight);

    DirectionalLight *directionalLight = new DirectionalLight(0.5,glm::vec3(1.0f, 0.95f, 0.8f),glm::normalize(glm::vec3(0.8f, 0.25f, 0.4f)));
    directionalLight->name = "DirectionalLight";
    root->adopt(directionalLight);

    /*
    PointLight *redLight = new PointLight({-10, 4, 0}, 0.1, {1, 0, 0}, 5, 2);
    redLight->name = "RedLight";
    root->adopt(redLight);

    PointLight *yellowLight = new PointLight({0, 4, 10}, 0.1, {1, 1, 0}, 5, 2);
    yellowLight->name = "YellowLight";
    root->adopt(yellowLight);

    PointLight *blueLight = new PointLight({0, 4, -10}, 0.1, {0, 0, 1}, 5, 2);
    blueLight->name = "BlueLight";
    root->adopt(blueLight);
    */

    // Planet
    MovingPlanetUpdate *planet = new MovingPlanetUpdate();
    planet->globalTranslate({0, 2, 0});
    planet->name = "Planet";
    root->adopt(planet);
    // Moving child
    PointLight *planetLight = new PointLight({0, 0, 0}, 0.075, {0.0, 0.5, 0.9}, 4, 2);
    planet->adopt(planetLight);
    // Moving statue
    Model3D *planetStatue = new Model3D("Statue.gltf", {0, 0, 0}, {0, 0, 0}, {1, 1, 1}, &mat1);
    planetStatue->name = "PlanetStatue";
    planet->adopt(planetStatue);
    // Planet child
    MovingPlanetChildUpdate *movingChild = new MovingPlanetChildUpdate();
    planet->adopt(movingChild);
    // Rotates around the planet
    Model3D *orbitingStatue = new Model3D("Statue.gltf", {0, 0, 0}, {0, 0, 0}, {1, 1, 1}, &mat1);
    orbitingStatue->localTranslate({1, 0, 0});
    movingChild->adopt(orbitingStatue);

    return root;
}

Node* createUnitScene() {
    Node *root = new Node();
    root->name = "root";

    root->adopt(PlayerNode::makeStandardPlayer());

    // Models
    Node *models = new Node();
    models->name = "ModelContainer";
    root->adopt(models);

    Model3D *plane = new Model3D("Unit Plane.gltf", {0, 0, 0}, {0, 0, 0}, {20, 1, 200}, &mat1);
    BoxCollider* planeColl = new BoxCollider(1.0f, 0.05f, 1.0f);
    planeColl->name = "PlaneHB";
    planeColl->movementStatus = STATIC;
    planeColl->layer = ENVIRONMENT;
    plane->name = "Plane";
    plane->adopt(planeColl);
    models->adopt(plane);

    for (int i = 0; i < 5; i++) {
        Model3D* cube = new Model3D("Unit Cube.gltf", {i, 2, 0}, {0, 0, 0}, {1, 1, 1}, TEX_MAT("cube.png"));
        models->adopt(cube);
        BoxCollider* box = new BoxCollider();
        cube->adopt(box);
        box->setLocalPosition(VEC3_ZERO);
   }
    Model3D* sphere = new Model3D("Unit Sphere.gltf", {0, 2, 3}, {0, 0, 0}, {1, 1, 1}, &outlineMat);
    outlineMat.toggle();
    models->adopt(sphere);
    SphereCollider* sColl = new SphereCollider();
    sphere->adopt(sColl);
    sColl->setLocalPosition(VEC3_ZERO);
    Model3D* cap = new Model3D("Unit Capsule.gltf", {0, 2, 5}, {0, 0, 0}, {1, 1, 1}, &blankMat);
    models->adopt(cap);
    CapsuleCollider* cColl = new CapsuleCollider();
    cap->adopt(cColl);
    cColl->setLocalPosition(VEC3_ZERO);

    Model3D* mage = new Model3D("Mage.gltf", {0, 0, 7}, {0, 0, 0}, {1, 1, 1}, TEX_MAT("mage.png"));
    models->adopt(mage);

    Model3D* tree = new Model3D("Spooky Tree.gltf", {0, 0, 10}, {0, 0, 0}, VEC3_ONE, &treeMat);
    models->adopt(tree);

    Model3D* grass = new Model3D("Grass.gltf", {0, 0, 12}, {0, 0, 0}, VEC3_ONE, TEX_MAT("grass.png"));
    models->adopt(grass);

    Model3D* bridge = new Model3D("Bridge.gltf", {0, 0, 15}, {0, 0, 0}, VEC3_ONE, TEX_MAT("wood-metal.png"));
    models->adopt(bridge);

    Node3D* lever = new Node3D();
    Model3D* lever_b = new Model3D("Lever_Base.gltf", {0, 0, 0}, {0, 0, 0}, VEC3_ONE, TEX_MAT("wood-metal.png"));
    lever->adopt(lever_b);
    Model3D* lever_h = new Model3D("Lever_Handle.gltf", {0, 0, 0}, {0, 0, 0}, VEC3_ONE, TEX_MAT("wood-metal.png"));
    lever->adopt(lever_h);
    lever->globalTranslate({0, 0, 17});
    models->adopt(lever);

    Model3D* bones = new Model3D("Pile of Bones.gltf", {0, 0, 20}, {0, 0, 0}, VEC3_ONE, &blankMat);
    models->adopt(bones);

    Model3D* coffin = new Model3D("Coffin.gltf", {0, 0, 23}, {0, 0, 0}, VEC3_ONE, &blankMat);
    models->adopt(coffin);

    Model3D* chains = new Model3D("Chains.gltf", {0, 5, 26}, {0, 0, 0}, VEC3_ONE, &blankMat);
    models->adopt(chains);

    Model3D* hook = new Model3D("Hook and Chain.gltf", {0, 5, 29}, {0, 0, 0}, VEC3_ONE, &blankMat);
    models->adopt(hook);

    Model3D* cage = new Model3D("Cage.gltf", {0, 5, 32}, {0, 0, 0}, VEC3_ONE, &blankMat);
    models->adopt(cage);

    Model3D* torch = new Model3D("Torch.gltf", {0, 5, 35}, {0, 0, 0}, VEC3_ONE, &blankMat);
    models->adopt(torch);

    Model3D* skele1 = new Model3D("Skeleton_crawl.gltf", {0, 0, 40}, {0, 0, 0}, VEC3_ONE, &blankMat);
    models->adopt(skele1);

    Model3D* skele2 = new Model3D("Skeleton_lean.gltf", {0, 0, 45}, {0, 0, 0}, VEC3_ONE, &blankMat);
    models->adopt(skele2);

    // Lights
    AmbientLight *ambientLight = new AmbientLight({0.08f, 0.14f, 0.20f},{0.035f, 0.04f, 0.045f}, {0.0f, 1.0f, 0.0f});
    ambientLight->name = "AmbientLight";
    root->adopt(ambientLight);

    DirectionalLight *directionalLight = new DirectionalLight(0.5,glm::vec3(1.0f, 0.95f, 0.8f),glm::normalize(glm::vec3(0.8f, 0.25f, 0.4f)));
    directionalLight->name = "DirectionalLight";
    root->adopt(directionalLight);

    return root;
}

Node* createForestScene() {
    Node *root = new Node();
    root->name = "root";

    // Map
    OrthoCamera *mapCam = new OrthoCamera(-32, 32, -16, 16, -10, 200);
    mapCam->setGlobalPosition({0, 10, 0});
    // root->adopt(mapCam);

    Node3D *player = PlayerNode::makeStandardPlayer(mapCam);
    player->adopt(mapCam);
    root->adopt(player);

    root->adopt(new FPSTextUpdater());

    root->adopt(new AudioController(new AudioNode("heavyRain.wav", 0.02f), true));

    // Models
    Node *models = new Node();
    models->name = "ModelContainer";
    root->adopt(models);

    Particles* rain = new Particles({0.0f,10.0f,0.0f}, {0.0f, 0.0f, 0.0f}, "", &rainMat);
    rain->setMaxParticles(25);
    rain->setBounds({10.0f, 5.0f, 10.0f});
    rain->setMaxLifeTime({0.5f, 1.0f});
    rain->setLinearVelocity({0.0f, -15.0f, 0.0f});
    rain->setTarget(player);
    models->adopt(rain);

    Model3D *plane = new Model3D("Unit Plane.gltf", {0, 0, 26.25f}, {0, 0, 0}, {100, 1, 100}, pathMat);
    BoxCollider* planeColl = new BoxCollider(1.0f, 0.05f, 1.0f);
    planeColl->movementStatus = STATIC;
    planeColl->layer = ENVIRONMENT;
    plane->adopt(planeColl);
    models->adopt(plane);

    Model3D *plane2 = new Model3D("Unit Plane.gltf", {0, 0, 141.5f}, {0, M_PI, 0}, {100, 1, 100}, pathMat);
    BoxCollider* plane2Coll = new BoxCollider(1.0f, 0.05f, 1.0f);
    plane2Coll->movementStatus = STATIC;
    plane2Coll->layer = ENVIRONMENT;
    plane2->adopt(plane2Coll);
    models->adopt(plane2);

    Collider* bridgeWall = new BoxCollider(10.0f, 4.0f, 1.0f);
    bridgeWall->name = "BridgeWall";
    bridgeWall->setGlobalPosition({0, 0, 76.525f});
    bridgeWall->movementStatus = STATIC;
    bridgeWall->layer = ENVIRONMENT;
    root->adopt(bridgeWall);

    Model3D* bridge = new Model3D("Bridge.gltf", {0, 0, 91.5f}, {0, M_PI, 0}, VEC3_ONE * 1.6f, wood_metalMat);
    AudioNode3D *bridgeSound = new AudioNode3D("loweringBridge.mp3", 4, 20, 2, INVERSE, 1);
    bridge->adopt(bridgeSound);
    models->adopt(bridge);

    AudioNode3D *leverSound = new AudioNode3D("leverPull.mp3", 1, 5, 2, INVERSE, 1);
    Node3D* lever = LeverNode::makeStandardLever(bridge, bridgeWall, leverSound, bridgeSound);
    lever->setGlobalPosition({9, 0, 74});
    lever->adopt(leverSound);
    lever->localRotateY(glm::radians(23.0f));
    root->adopt(lever);

    Model3D* castle = new Model3D("Castle.gltf", {0, 0, 105.0f}, {0, M_PI, 0}, VEC3_ONE, TEX_MAT("Diffuse_palette.jpg"));
    models->adopt(castle);
    Model3D* door = new Model3D("Door.gltf", VEC3_ZERO, VEC3_ZERO, VEC3_ONE, wood_metalMat);
    castle->adopt(door);

    Model3D* rocks1 = new Model3D("Unit Plane.gltf", {0, -2.5f, 91.5f}, {M_PI/2, M_PI, 0}, {100, 1, 5}, rockMat);
    models->adopt(rocks1);
    Model3D* rocks2 = new Model3D("Unit Plane.gltf", {0, -2.5f, 76.2f}, {M_PI/2, 0, 0}, {100, 1, 5}, rockMat);
    models->adopt(rocks2);

    //flames
    Model3D *fire1 = new Model3D("Unit Plane.gltf", {7.0f, 8.0f, 101.0f}, {glm::radians(90.0f), 0.0f, glm::radians(180.0f)}, glm::vec3(2.5, 3.5, 2.5), &flame1);
    models->adopt(fire1);

    PointLight* pl1 = new PointLight(glm::vec3(0.0f, 0.0f, 0.0f), 2.0f, glm::vec3(0.9f, 0.7f, 0.0f), 0.5f, 2.0f);
    fire1->adopt(pl1);

    Model3D *fire2 = new Model3D("Unit Plane.gltf", {-7.0f, 8.0f, 101.0f}, {glm::radians(90.0f), 0.0f, glm::radians(180.0f)}, glm::vec3(2.5, 3.5, 2.5), &flame1);
    models->adopt(fire2);

    PointLight* pl2 = new PointLight(glm::vec3(0.0f, 0.0f, 0.0f), 2.0f, glm::vec3(0.9f, 0.7f, 0.0f), 0.5f, 2.0f);
    fire2->adopt(pl2);

    Model3D* water = new Model3D("Water.gltf", {0, -3, 84}, VEC3_ZERO, {100, 100, 100}, &waterMat);
    models->adopt(water);

    // trees
    std::vector<glm::vec3> treePoints = {{45.616642, 0, 94.118866},{43.367878, 0, 94.396805},{38.698673, 0, 94.30226},{33.69174, 0, 93.731926},{29.39251, 0, 94.26331},{25.214565, 0, 93.677765},{21.393332, 0, 94.150055},{17.90622, 0, 94.581055},{14.781052, 0, 96.9688},{11.612019, 0, 99.93954},{10.108809, 0, 102.04704},{-13.3153105, 0, 101.73126},{-15.640373, 0, 98.559525},{-19.994165, 0, 95.86947},{-24.2057, 0, 93.71972},{-27.518608, 0, 93.81975},{-30.511477, 0, 93.4381},{-33.02612, 0, 94.09174},{-36.250507, 0, 93.6715},{-39.25984, 0, 93.25986},{-41.978317, 0, 93.966484},{-45.182198, 0, 93.62913},{-47.89809, 0, 94.335075},{-18.63687, 0, 97.51435},{43.30443, 0, 98.315094},{38.754086, 0, 97.97017},{36.235714, 0, 103.513016},{32.45458, 0, 98.85919},{29.651907, 0, 101.942055},{30.100758, 0, 106.854416},{26.02044, 0, 104.61031},{22.54952, 0, 99.4061},{19.182796, 0, 98.83391},{16.757782, 0, 102.761665},{18.353632, 0, 107.325386},{17.752792, 0, 112.21178},{11.848233, 0, 110.893486},{13.92058, 0, 104.09527},{21.29521, 0, 103.86214},{24.724155, 0, 108.72845},{-11.319699, 0, 105.825424},{-15.749709, 0, 107.441376},{-18.799364, 0, 104.88884},{-19.164137, 0, 101.27184},{-23.542393, 0, 98.03207},{-26.157658, 0, 95.74425},{-30.029087, 0, 97.01049},{-34.507442, 0, 96.94821},{-37.94153, 0, 98.83092},{-42.233772, 0, 95.80046},{-45.43914, 0, 98.02202},{-42.28768, 0, 99.54268},{-35.083076, 0, 102.28585},{-29.60458, 0, 101.42561},{-24.530281, 0, 101.2019},{-21.584696, 0, 103.63691},{-20.221403, 0, 110.235565},{-14.090034, 0, 111.093056},{-12.378886, 0, 112.20315},{-24.703949, 0, 107.55342},{-26.911108, 0, 98.99473},{27.04289, 0, 98.16005},{-1.4900031, 0, -7.4666696},{-4.4717407, 0, -6.031718},{-8.147978, 0, -3.580738},{-10.602313, 0, -1.393214},{-9.764701, 0, 0.767875},{-9.589022, 0, 3.6422274},{-7.553951, 0, 5.9777937},{-6.3217177, 0, 8.688609},{-5.7114954, 0, 12.880987},{-5.5053144, 0, 16.703545},{-4.7294745, 0, 19.990808},{-5.131528, 0, 23.65886},{-5.42357, 0, 26.877836},{-6.032557, 0, 30.430645},{-6.3090844, 0, 33.90326},{-6.85261, 0, 37.44869},{-6.4628415, 0, 40.94384},{-6.26641, 0, 44.5801},{-6.5654893, 0, 47.67282},{-6.875375, 0, 50.579926},{-7.297063, 0, 53.87048},{-7.709286, 0, 57.348423},{-8.163952, 0, 60.07494},{-9.345858, 0, 62.902946},{-10.04399, 0, 66.019615},{-11.607886, 0, 69.18325},{-13.532837, 0, 71.799355},{-16.656973, 0, 74.4818},{-19.253984, 0, 75.87457},{-21.452354, 0, 76.007126},{-24.019505, 0, 75.77207},{-26.118036, 0, 75.84713},{-28.63543, 0, 75.61665},{-31.097372, 0, 75.85922},{-33.86248, 0, 75.60609},{-36.403374, 0, 75.63948},{-39.71114, 0, 75.336655},{-41.809048, 0, 75.41084},{-44.270298, 0, 75.653694},{-46.34424, 0, 75.463806},{-49.0102, 0, 75.21974},{47.273895, 0, 75.670456},{43.63944, 0, 75.76295},{39.9246, 0, 75.390755},{35.948547, 0, 75.02349},{31.850325, 0, 75.12774},{28.016043, 0, 75.2253},{24.182518, 0, 75.32285},{20.576504, 0, 73.94949},{16.422234, 0, 71.85604},{12.64167, 0, 68.82464},{9.759885, 0, 66.44759},{9.3373995, 0, 63.02035},{8.853088, 0, 59.40179},{7.3523927, 0, 53.49054},{7.281336, 0, 49.49707},{6.7269197, 0, 45.369324},{6.871913, 0, 41.94887},{7.104684, 0, 38.18357},{7.6438265, 0, 35.17796},{8.170417, 0, 32.10619},{7.254827, 0, 29.294895},{5.6194463, 0, 26.645168},{6.7455235, 0, 24.023676},{5.7489996, 0, 20.093536},{5.8929005, 0, 16.672071},{6.0555153, 0, 13.38281},{6.202599, 0, 9.495374},{8.456965, 0, 6.2582855},{9.08316, 0, 2.904467},{8.304268, 0, -0.39158508},{6.3752275, 0, -3.2675736},{3.5540397, 0, -6.7555957},{5.151063, 0, -6.088594},{8.860793, 0, 55.758366}};
    for (glm::vec3 p : treePoints) {
        float yRot = 2 * M_PI * randNorm();
        float scale = 0.9f + 0.6f * randNorm();
        Model3D* tree = new Model3D("Spooky Tree.gltf", p, {0, yRot, 0}, VEC3_ONE * scale, &treeMat);
        models->adopt(tree);
    }
    constexpr int TREE_NUM = 100;
    glm::vec3 corner1 = {11, 0, 7.5f};
    glm::vec3 corner2 = {50, 0, 76.25f};
    glm::vec3 delta = corner2 - corner1;
    for (int i = 0; i < TREE_NUM; ++i)
    {
        glm::vec3 p = corner1 + delta * glm::vec3(randNorm(), randNorm(), randNorm());
        float yRot = 2 * M_PI * randNorm();
        float scale = 1.2f + 0.6f * randNorm();
        Model3D* tree = new Model3D("Spooky Tree.gltf", p, {0, yRot, 0}, VEC3_ONE * scale, &treeMat);
        models->adopt(tree);
    }
    corner1 = {-11, 0, 7.5f};
    corner2 = {-50, 0, 76.25f};
    delta = corner2 - corner1;
    for (int i = 0; i < TREE_NUM; ++i)
    {
        glm::vec3 p = corner1 + delta * glm::vec3(randNorm(), randNorm(), randNorm());
        float yRot = 2 * M_PI * randNorm();
        float scale = 1.2f + 0.6f * randNorm();
        Model3D* tree = new Model3D("Spooky Tree.gltf", p, {0, yRot, 0}, VEC3_ONE * scale, &treeMat);
        models->adopt(tree);
    }
    corner1 = {-50, 0, 7.5f};
    corner2 = {50, 0, -23.75f};
    delta = corner2 - corner1;
    for (int i = 0; i < TREE_NUM; ++i)
    {
        glm::vec3 p = corner1 + delta * glm::vec3(randNorm(), randNorm(), randNorm());
        if (glm::length(p) <= 13.0f) continue; //Too close to origin
        float yRot = 2 * M_PI * randNorm();
        float scale = 1.2f + 0.6f * randNorm();
        Model3D* tree = new Model3D("Spooky Tree.gltf", p, {0, yRot, 0}, VEC3_ONE * scale, &treeMat);
        models->adopt(tree);
    }
    // Grass
    constexpr int GRASS_NUM = 400;
    corner1 = {-50, 0, 76.25f};
    corner2 = {50, 0, -23.75f};
    delta = corner2 - corner1;
    for (int i = 0; i < GRASS_NUM; ++i) {
        glm::vec3 p = corner1 + delta * glm::vec3(randNorm(), randNorm(), randNorm());
        float yRot = 2 * M_PI * randNorm();
        float scale = 1.0f + 1.5f * randNorm();
        Model3D* tree = new Model3D("Grass.gltf", p, {0, yRot, 0}, VEC3_ONE * scale, grassMat);
        models->adopt(tree);
    }

    // Walls
    Node* walls = new Node();
    root->adopt(walls);

    std::vector<std::vector<glm::vec3>> wallDefs = {
        {{6.727, 2.000, 35.919}, {0.000, 0.000, 0.000}, {1.000, 2.000, 30.000}},
        {{-6.589, 2.000, 35.919}, {0.000, 0.000, 0.000}, {1.000, 2.000, 30.000}},
        {{-11.696, 2.000, 71.606}, {0.000, 0.888, 0.000}, {8.000, 2.000, 1.000}},
        {{11.728, 2.000, 69.981}, {0.000, 2.336, 0.000}, {8.000, 2.000, 1.000}},
        {{14.355, 2.000, 97.026}, {0.000, 0.812, 0.000}, {8.000, 2.000, 1.000}},
        {{-14.523, 2.000, 96.686}, {0.000, -0.931, 0.000}, {8.000, 2.000, 1.000}},
        {{6.100, 2.000, -4.702}, {0.000, -0.744, 0.000}, {8.964, 2.000, 1.000}},
        {{-6.502, 2.000, -4.107}, {0.000, 0.842, 0.000}, {8.425, 2.000, 1.000}},
        {{8.958, 2.000, 3.726}, {0.000, 0.990, 0.000}, {3.478, 2.000, 1.000}},
        {{9.882, 2.000, 76.525}, {0.000, 0.000, 0.000}, {6.499, 2.000, 1.000}},
        {{-11.496, 2.000, 76.525}, {0.000, 0.000, 0.000}, {6.499, 2.000, 1.000}},
        {{9.882, 2.000, 92.114}, {0.000, 0.000, 0.000}, {6.499, 2.000, 1.000}},
        {{-11.496, 2.000, 92.114}, {0.000, 0.000, 0.000}, {6.499, 2.000, 1.000}},
        {{-9.260, 2.000, 4.577}, {0.000, 2.273, 0.000}, {3.478, 2.000, 1.000}},
        {{0.000, -1.000, 83.317}, {0.000, 0.000, 0.000}, {5.445, 1.000, 8.436}},
        {{6.510, 2.000, 84.204}, {0.000, 1.571, 0.000}, {8.075, 2.000, 1.000}},
        {{-6.921, 2.000, 84.204}, {0.000, 1.571, 0.000}, {8.075, 2.000, 1.000}}
    };
    for (auto w : wallDefs) {
        BoxCollider* coll = new BoxCollider(w[0], w[1], w[2] * 2.0f);
        coll->name = std::format("Wall @ {}", w[0]);
        coll->movementStatus = STATIC;
        coll->layer = ENVIRONMENT;
        walls->adopt(coll);
        /*
        Model3D* box = new Model3D("Unit Cube.gltf", VEC3_ZERO, VEC3_ZERO, VEC3_ONE, &blankMat);
        coll->adopt(box);
        */
    }

    BoxCollider* endZone = new BoxCollider(true, {0, 2, 100}, VEC3_ZERO, {20, 4, 1});
    endZone->movementStatus = STATIC;
    endZone->layer = ENVIRONMENT;
    endZone->onTriggerEnter = [](Collider* _)
    {
        info("You have reached the door to the castle, loading next scene...");
        //TODO Load scene (set to Dark to test)
        Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("Dark")));
    };
    walls->adopt(endZone);

    AmbientLight *ambientLight = new AmbientLight({0.18, 0.106, 0.341}, {0.01, 0.01, 0.2}, VEC3_Y);//new AmbientLight({0.08f, 0.14f, 0.20f},{0.035f, 0.04f, 0.045f}, {0.0f, 1.0f, 0.0f});
    ambientLight->name = "AmbientLight";
    root->adopt(ambientLight);

    /*
    DirectionalLight *directionalLight = new DirectionalLight(0.1,glm::vec3(1.0f, 0.95f, 0.8f),glm::normalize(glm::vec3(0.8f, 0.25f, 0.4f)));
    root->adopt(directionalLight);
    */


    LightningNode* lightning = new LightningNode(
        new DirectionalLight(10,glm::vec3(1.0f, 0.95f, 0.8f),glm::normalize(glm::vec3(0.8f, 0.25f, 0.4f))),
        new AudioNode("lightning.wav", 0.2f),
        10.0f, 7.0f, 3.0f
        );
    root->adopt(lightning);


    // TutorialText
    Text2D *cameraTutorial = new Text2D("Press M to change camera", {-1, 1}, "SS", false, false, false, TAL_LEFT, TRH_LEFT, TRV_BOTTOM);
    root->adopt(cameraTutorial);

    return root;
}

Node* createDarkScene() {
    Node *root = new Node();
    root->name = "root";

    // Player
    Node3D* player = PlayerNode::makeStandardPlayer();
    static_cast<CapsuleCollider*>(player)->radius = 3;
    root->adopt(player);

    // Audio
    AudioNode3D *rain = new AudioNode3D("heavyRain.wav", 0.02f);
    rain->localTranslate({0, 4, 0});
    player->adopt(new AudioController(rain, true));

    Node *staticObjects = new Node();
    staticObjects->name = "StaticObjectsContainer";
    root->adopt(staticObjects);

    float cellSize = 8;

    #define LABSIZE 12
    const char labyrinth[LABSIZE][LABSIZE] = {
        {'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'},
        {'W', '.', '.', '.', 'W', '.', '.', '.', 'W', '.', '.', 'W'},
        {'W', 'W', 'W', '.', 'W', '.', 'W', '.', 'W', '.', 'W', 'W'},
        {'W', '.', '.', '.', '.', '.', 'W', '.', '.', '.', '.', 'W'},
        {'W', '.', 'W', 'W', 'W', 'W', 'W', '.', 'W', 'W', '.', 'W'},
        {'W', '.', '.', '.', 'W', '.', '.', '.', 'W', 'W', '.', 'W'},
        {'W', 'W', 'W', '.', 'W', '.', 'W', 'W', '.', 'W', '.', 'W'},
        {'W', 'S', 'W', '.', '.', '.', 'W', '.', '.', '.', '.', 'W'},
        {'W', '.', 'W', 'W', 'W', '.', 'W', '.', 'W', '.', 'W', 'W'},
        {'W', '.', '.', '.', '.', '.', '.', 'W', '.', '.', '.', 'W'},
        {'W', '.', 'W', '.', 'W', 'W', '.', '.', 'W', 'W', '.', 'E'},
        {'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'},
    };

    glm::vec3 offset[4] = {
        {0, 0, cellSize/2.0f},
        {0, 0, -cellSize/2.0f},
        {cellSize/2.0f, 0, 0},
        {-cellSize/2.0f, 0, 0}
    };

    glm::vec3 rotations[4] = {
        {std::numbers::pi/2, 0, 0},
        {std::numbers::pi/2, 0, std::numbers::pi},
        {std::numbers::pi/2, std::numbers::pi/2, 0},
        {std::numbers::pi/2, -std::numbers::pi/2, 0}
    };

    // Floor
    Model3D *floor = new Model3D("Water.gltf", {cellSize * LABSIZE/2, 0, cellSize * LABSIZE/2}, {0, 0, 0}, {cellSize * LABSIZE/2, 1, cellSize * LABSIZE/2}, &sMat);
    floor->name = "Floor";
    staticObjects->adopt(floor);
    BoxCollider* floorCollider = new BoxCollider(5.0f, 0.2f, 5.0f);
    floorCollider->name = "Floorcoll";
    floorCollider->movementStatus = STATIC;
    floorCollider->layer = ENVIRONMENT;
    floor->adopt(floorCollider);

    // Ceiling
    Model3D *ceiling = new Model3D("Water.gltf", {cellSize * LABSIZE/2, cellSize, cellSize * LABSIZE/2}, {std::numbers::pi, 0, 0}, {cellSize * LABSIZE/2, 1, cellSize * LABSIZE/2}, &sMat);
    ceiling->name = "Floor";
    staticObjects->adopt(ceiling);

    // Walls
    for (int i = 0; i < LABSIZE; i++) {
        for (int j = 0; j < LABSIZE; j++) {
            switch(labyrinth[i][j]) {
            case '.': {
                break;
            }
            case 'W': {
                const glm::vec3 pos = glm::vec3(i*cellSize, cellSize/2, j*cellSize);

                for (int k = 0; k < 4; k++) {
                    Model3D *wall = new Model3D("Water.gltf", pos + offset[k], rotations[k], {cellSize/2, 1, cellSize/2}, &sMat);
                    wall->name = std::format("Wall {} {} {}", i, j, k);
                    staticObjects->adopt(wall);
                }

                // Colliders tank the FPS
                BoxCollider* wallCollider = new BoxCollider(pos, {0, 0, 0}, {cellSize/2, cellSize/2, cellSize/2});
                wallCollider->name = std::format("WallColl {} {}", i, j);
                wallCollider->movementStatus = STATIC;
                wallCollider->layer = ENVIRONMENT;
                staticObjects->adopt(wallCollider);
                break;
            }
            case 'S':  {
                // Maybe add a room before the maze? (and after?)
                player->setGlobalPosition({i*cellSize, 5, j*cellSize});
                break;
            }
            case 'E': {
                // end room
                Node* endRoom = new Node();

                Model3D *endFloor = new Model3D("Unit Plane.gltf", {i*cellSize, 0, j*cellSize + 20.5}, {0, 0, 0}, {25, 1, 25}, &mat1);
                BoxCollider* endFloorColl = new BoxCollider(1.0f, 0.05f, 1.0f);
                endFloorColl->name = "endFloorColl";
                endFloorColl->movementStatus = STATIC;
                endFloorColl->layer = ENVIRONMENT;
                endFloor->adopt(endFloorColl);
                endRoom->adopt(endFloor);

                Model3D *endWall1 = new Model3D("Unit Plane.gltf", {i*cellSize, 5, j*cellSize + 30}, {std::numbers::pi / 2, 0, std::numbers::pi}, {25, 1, 25}, &mat1);
                BoxCollider* endWall1Coll = new BoxCollider({i*cellSize, 5, j*cellSize + 30}, VEC3_ZERO, {25.0f, 25.0f, 1.0f});
                endWall1Coll->name = "endWall1Coll";
                endWall1Coll->movementStatus = STATIC;
                endWall1Coll->layer = ENVIRONMENT;
                endWall1->adopt(endWall1Coll);
                endRoom->adopt(endWall1);
                Model3D *endWall2 = new Model3D("Unit Plane.gltf", {i*cellSize - 9.5, 5, j*cellSize + 17.5}, {std::numbers::pi / 2, std::numbers::pi / 2, 0}, {27, 1, 27}, &mat1);
                BoxCollider* endWall2Coll = new BoxCollider({i*cellSize - 9.5, 5, j*cellSize + 17.5}, VEC3_ZERO, {1.0f, 25.0f, 25.0f});
                endWall2Coll->name = "endWall2Coll";
                endWall2Coll->movementStatus = STATIC;
                endWall2Coll->layer = ENVIRONMENT;
                endWall2->adopt(endWall2Coll);
                endRoom->adopt(endWall2);
                Model3D *endWall3 = new Model3D("Unit Plane.gltf", {i*cellSize + 9.5, 5, j*cellSize + 17.5}, {std::numbers::pi / 2, -std::numbers::pi / 2, 0}, {27, 1, 27}, &mat1);
                BoxCollider* endWall3Coll = new BoxCollider({i*cellSize + 9.5, 5, j*cellSize + 17.5}, VEC3_ZERO, {1.0f, 25.0f, 25.0f});
                endWall3Coll->name = "endWall3Coll";
                endWall3Coll->movementStatus = STATIC;
                endWall3Coll->layer = ENVIRONMENT;
                endWall3->adopt(endWall3Coll);
                endRoom->adopt(endWall3);

                Model3D *endCeiling = new Model3D("Unit Plane.gltf", {i*cellSize, cellSize, j*cellSize + 20.5}, {M_PI, 0, 0}, {25, 1, 25}, &rMat);
                endRoom->adopt(endCeiling);
    
                Model3D *magicCircle = new Model3D("Water.gltf", {i*cellSize, 0.1f, j*cellSize + 20.5}, {0, 0, 0}, {3, 1, 3}, &magicMat);
                endRoom->adopt(magicCircle);

                BoxCollider* endZone = new BoxCollider(true, {i*cellSize, 0.1f, j*cellSize + 20.5}, VEC3_ZERO, {3, 3, 3});
                endZone->movementStatus = STATIC;
                endZone->layer = ENVIRONMENT;
                endZone->onTriggerEnter = [](Collider* _)
                {
                    Engine::endGame();
                };
                endRoom->adopt(endZone);

                root->adopt(endRoom);
                break;
            }
            default: {
                error("What?");
            }
            }
        }
    }

    // Lights
    AmbientLight *ambientLight = new AmbientLight({0.08f, 0.14f, 0.20f},{0.035f, 0.04f, 0.045f}, {0.0f, 1.0f, 0.0f});
    ambientLight->name = "AmbientLight";
    root->adopt(ambientLight);

    DirectionalLight *directionalLight = new DirectionalLight(0.5,glm::vec3(1.0f, 0.95f, 0.8f),glm::normalize(glm::vec3(0.8f, 0.25f, 0.4f)));
    directionalLight->name = "DirectionalLight";
    root->adopt(directionalLight);

    // FPS
    root->adopt(new FPSTextUpdater());

    // TutorialText
    Text2D *cameraTutorial = new Text2D("Press left mouse to cast spell", {-1, 1}, "SS", false, false, false, TAL_LEFT, TRH_LEFT, TRV_BOTTOM);
    root->adopt(cameraTutorial);

    return root;
}

Node* createLabyrinthScene()
{
    Node* root = new Node();
    root->name = "root";

    Node3D* player = PlayerNode::makeStandardPlayer();
    player->setGlobalPosition({0, 3, -7});
    root->adopt(player);

    // temp floor, add holes later
    Model3D* labFloor = new Model3D("Unit Plane.gltf", {0, 0, 40}, VEC3_ZERO, {90, 1, 80}, &sMat);
    BoxCollider* floorColl = new BoxCollider(1, 0.05f, 1);
    floorColl->name = "floorCollider";
    floorColl->movementStatus = STATIC;
    floorColl->layer = ENVIRONMENT;
    labFloor->adopt(floorColl);
    root->adopt(labFloor);

    Model3D* labCeiling = new Model3D("Unit Plane.gltf", {0, 6, 40}, {0, 0, M_PI}, {90, 1, 80}, &sMat);
    root->adopt(labCeiling);

    // Start and end rooms
    std::vector<std::vector<glm::vec3>> objPoints = {
        {{0.000, -0.010, -4.900}, {0.000, 0.000, 0.000}, {10.000, 1.000, 10.000}},
        {{-5.000, 3.000, -4.900}, {0.000, 3.142, 1.571}, {6.000, 1.000, 10.000}},
        {{5.000, 3.000, -4.900}, {3.142, 0, 1.571}, {6.000, 1.000, 10.000}},
        {{0.000, 3.000, -9.900}, {0, 1.571, 1.571}, {6.000, 1.000, 10.000}},

        {{-0.000, -0.010, 84.910}, {0.000, 3.142, 0.000}, {10.000, 1.000, 10.000}},
        {{5.000, 3.000, 84.910}, {3.142, 0.000, 1.571}, {6.000, 1.000, 10.000}}, // ok
        {{-5.000, 3.000, 84.910}, {0.000, 0.000, -1.571}, {6.000, 1.000, 10.000}}, // flip
        {{0.000, 3.000, 89.910}, {0.000, -1.571, 1.571}, {6.000, 1.000, 10.000}} // rotate
    };
    for (auto p : objPoints)
    {
        Model3D* plane = new Model3D("Unit Plane.gltf", p[0], p[1], p[2], rockMat);
        BoxCollider* coll = new BoxCollider(1, 0.1f, 1);
        coll->name = std::format("room plane @ {}", p[0]);
        coll->movementStatus = STATIC;
        coll->layer = ENVIRONMENT;
        plane->adopt(coll);
        root->adopt(plane);
    }
    objPoints.clear();
    
    Model3D* startCeiling = new Model3D("Unit Plane.gltf", {0, 5.990, -4.900}, {0, 0, 3.142}, {10, 1, 10}, rockMat);
    root->adopt(startCeiling);
    Model3D* endCeiling = new Model3D("Unit Plane.gltf", {0, 5.990, 84.900}, {0, 0, 3.142}, {10, 1, 10}, &rMat);
    root->adopt(endCeiling);
    Model3D* magicCircle = new Model3D("Unit Plane.gltf", {0, 0, 84.900}, VEC3_ZERO, {5, 1, 5}, &magicMat);
    root->adopt(magicCircle);

    // Walls
    objPoints = {
        {{-25.198, 3.000, -0.372}, {42.000, 6.000, 2.000}},
        {{24.802, 3.000, -0.372}, {42.000, 6.000, 2.000}},
        {{-25.198, 3.000, 79.628}, {42.000, 6.000, 2.000}},
        {{24.802, 3.000, 79.628}, {42.000, 6.000, 2.000}},
        {{-45.198, 3.000, 39.628}, {2.000, 6.000, 82.000}},
        {{44.802, 3.000, 39.628}, {2.000, 6.000, 82.000}},
        {{-25.198, 3.000, 9.628}, {2.000, 6.000, 22.000}},
        {{-25.198, 3.000, 44.628}, {2.000, 6.000, 12.000}},
        {{-25.198, 3.000, 69.628}, {2.000, 6.000, 22.080}},
        {{-15.198, 3.000, 24.628}, {2.000, 6.000, 32.076}},
        {{-5.198, 3.000, 34.628}, {2.000, 6.000, 32.076}},
        {{-5.198, 3.000, 74.628}, {2.000, 6.000, 12.000}},
        {{4.802, 3.000, 64.628}, {2.000, 6.000, 12.000}},
        {{4.802, 3.000, 44.628}, {2.000, 6.000, 12.000}},
        {{14.802, 3.000, 44.628}, {2.000, 6.000, 12.000}},
        {{14.802, 3.000, 14.628}, {2.000, 6.000, 32.076}},
        {{24.802, 3.000, 44.628}, {2.000, 6.000, 32.076}},
        {{24.802, 3.000, 14.628}, {2.000, 6.000, 12.000}},
        {{34.802, 3.000, 14.628}, {2.000, 6.000, 12.000}},
        {{34.802, 3.000, 44.628}, {2.000, 6.000, 12.000}},
        {{34.802, 3.000, 64.628}, {2.000, 6.000, 12.000}},
        {{24.802, 3.000, 74.628}, {2.000, 6.000, 12.000}},
        {{-35.198, 3.000, 34.628}, {2.000, 6.000, 12.000}},
        {{-20.198, 3.000, 19.628}, {32.076, 6.000, 2.000}},
        {{-40.198, 3.000, 49.628}, {12.000, 6.000, 2.000}},
        {{-35.198, 3.000, 29.628}, {22.000, 6.000, 2.000}},
        {{-5.198, 3.000, 9.628}, {22.000, 6.000, 2.000}},
        {{14.802, 3.000, 19.628}, {22.000, 6.000, 2.000}},
        {{39.802, 3.000, 19.628}, {12.000, 6.000, 2.000}},
        {{-0.198, 3.000, 29.628}, {12.000, 6.000, 2.000}},
        {{24.802, 3.000, 29.628}, {22.000, 6.000, 2.000}},
        {{-20.198, 3.000, 39.628}, {12.000, 6.000, 2.000}},
        {{-40.198, 3.000, 9.628}, {12.000, 6.000, 2.000}},
        {{-10.198, 3.000, 49.628}, {12.000, 6.000, 2.000}},
        {{-25.198, 3.000, 59.628}, {22.000, 6.000, 2.000}},
        {{-30.198, 3.000, 69.628}, {12.000, 6.000, 2.000}},
        {{-0.198, 3.000, 59.628}, {12.000, 6.000, 2.000}},
        {{-0.198, 3.000, 69.628}, {32.076, 6.000, 2.000}},
        {{29.802, 3.000, 69.628}, {12.000, 6.000, 2.000}},
        {{39.802, 3.000, 39.628}, {12.000, 6.000, 2.000}},
        {{19.802, 3.000, 59.628}, {12.000, 6.000, 2.000}},
        {{9.802, 3.000, 39.628}, {12.000, 6.000, 2.000}},
        {{19.802, 3.000, 49.628}, {12.000, 6.000, 2.000}}
    };
    for (auto w : objPoints)
    {
        Model3D* wall = new Model3D("Unit Cube.gltf", w[0], VEC3_ZERO, w[1], &sMat);
        BoxCollider* coll = new BoxCollider();
        coll->name = std::format("wall @ {}", w[0]);
        coll->movementStatus = STATIC;
        coll->layer = ENVIRONMENT;
        wall->adopt(coll);
        root->adopt(wall);
    }
    objPoints.clear();
    
    // Decor
    Node* decor = new Node();
    decor->name = "Decor container";
    root->adopt(decor);
    // Bones
    objPoints = {
        {{-12.562, 0.000, 6.933}}, 
        {{10.930, 0.000, 3.018}}, 
        {{-8.891, 0.000, 16.151}}, 
        {{7.586, 0.000, 25.450}}, 
        {{-11.012, 0.000, 40.540}}, 
        {{-30.507, 0.000, 38.991}}, 
        {{-19.658, 0.000, 24.308}}, 
        {{-42.335, 0.000, 16.477}}, 
        {{-28.794, 0.000, 2.610}}, 
        {{-42.253, 0.000, 42.335}}, 
        {{-42.253, 0.000, 56.202}}, 
        {{-29.610, 0.000, 63.380}}, 
        {{-42.824, 0.000, 72.516}}, 
        {{-30.344, 0.000, 75.697}}, 
        {{-4.894, 0.000, 63.788}}, 
        {{-22.024, 0.000, 64.930}}, 
        {{-10.767, 0.000, 77.165}}, 
        {{13.133, 0.000, 63.869}}, 
        {{21.371, 0.000, 74.963}}, 
        {{21.127, 0.000, 55.305}}, 
        {{10.196, 0.000, 46.087}}, 
        {{21.616, 0.000, 33.525}}, 
        {{30.670, 0.000, 24.960}}, 
        {{18.924, 0.000, 23.737}}, 
        {{18.924, 0.000, 12.399}}, 
        {{39.072, 0.000, 6.118}}, 
        {{41.111, 0.000, 14.193}}, 
        {{39.317, 0.000, 35.401}}, 
        {{31.649, 0.000, 55.712}}, 
        {{40.214, 0.000, 47.555}}
    };
    for (auto p : objPoints)
    {
        const float rot = MA_TAU * randNorm();
        const float scale = 0.8f + 0.6f * randNorm();
        Model3D* obj = new Model3D("Pile of Bones.gltf", p[0], {0, rot, 0}, VEC3_ONE * scale, &sMat);
        decor->adopt(obj);
    }
    objPoints.clear();

    // Chains
    objPoints = {
        {{10.686, 6.000, 16.803}},
        {{9.299, 6.000, 16.396}},
        {{-2.529, 6.000, 25.450}},
        {{-1.224, 6.000, 26.755}},
        {{-18.598, 6.000, 28.142}},
        {{-27.081, 6.000, 35.238}},
        {{-39.806, 6.000, 46.169}},
        {{-40.051, 6.000, 59.791}},
        {{-35.075, 6.000, 65.011}},
        {{-40.703, 6.000, 67.703}},
        {{-20.637, 6.000, 72.597}},
        {{-11.175, 6.000, 63.869}},
        {{16.967, 6.000, 69.008}},
        {{22.024, 6.000, 68.927}},
        {{27.000, 6.000, 47.229}},
        {{42.253, 6.000, 59.057}},
        {{36.707, 6.000, 66.643}},
        {{30.263, 6.000, 72.108}},
        {{42.335, 6.000, 29.447}},
        {{23.003, 6.000, 22.269}},
        {{20.637, 6.000, 2.366}},
        {{42.498, 6.000, 2.855}},
        {{42.580, 6.000, 9.462}}
    };
    for (auto p : objPoints)
    {
        const float scale = 1 + randNorm();
        Model3D* obj = new Model3D("Chains.gltf", p[0], VEC3_ZERO, VEC3_ONE * scale, &sMat);
        decor->adopt(obj);
    }
    objPoints.clear();

    // Hooks
    objPoints = {
        {{10.359, 6.000, 15.580}},
        {{24.308, 6.000, 22.106}},
        {{40.133, 6.000, 1.795}},
        {{-17.130, 6.000, 27.244}},
        {{-28.550, 6.000, 7.178}},
        {{-28.305, 6.000, 48.861}},
        {{-18.924, 6.000, 72.026}},
        {{32.873, 6.000, 46.740}},
        {{21.861, 6.000, 39.480}}
    };
    for (auto p : objPoints)
    {
        const float rot = MA_TAU * randNorm();
        Model3D* obj = new Model3D("Hook and Chain.gltf", p[0], {0, rot, 0}, VEC3_ONE, &sMat);
        decor->adopt(obj);
    }
    objPoints.clear();

    // Cages
    objPoints = {
        {{12.303, 6.000, 22.228}},
        {{-16.450, 6.000, 42.552}},
        {{-36.598, 6.000, 67.839}},
        {{12.263, 6.000, 42.063}},
        {{37.386, 6.000, 57.480}}
    };
    for (auto p : objPoints)
    {
        Model3D* obj = new Model3D("Cage.gltf", p[0], VEC3_ZERO, VEC3_ONE, &sMat);
        decor->adopt(obj);
    }
    objPoints.clear();

    // Coffins
    objPoints = {
        {{-13.291, 1.328, 11.263}, {1.136, 0.462, -0.207}},
        {{17.807, 0.000, 25.835}, {0.000, 1.951, -0.000}},
        {{-32.950, 0.578, 32.054}, {0.544, 0.423, 0.251}},
        {{-2.493, 0.000, 32.544}, {0.000, 2.189, -0.000}},
        {{33.075, 1.474, 68.235}, {1.548, -2.590, -0.005}},
        {{-7.074, 1.271, 71.234}, {1.163, -0.886, 0.168}}
    };
    for (auto p : objPoints)
    {
        Model3D* obj = new Model3D("Coffin.gltf", p[0], p[1], VEC3_ONE, &sMat);
        decor->adopt(obj);
    }
    objPoints.clear();

    // Skeletons crawl
    objPoints = {
        {{6.315, 0.000, 12.444}, {0.000, 2.334,0.000}},
        {{7.573, 0.000, 32.369}, {0.000, 2.965, 0.000}},
        {{21.835, 0.000, 65.821}, {0.000, -1.770, 0.000}},
    };
    for (auto p : objPoints)
    {
        Model3D* obj = new Model3D("Skeleton_crawl.gltf", p[0], p[1], VEC3_ONE, &sMat);
        decor->adopt(obj);
    }
    objPoints.clear();

    // Skeletons lean
    objPoints = {
        {{-20.277, 0.000, 18.089}, {0.000, 2.869, 0.000}},
        {{-43.763, 0.000, 5.202}, {0.000, 1.898, 0.000}},
        {{-43.397, 0.000, 31.281}, {0.000, 0.830, 0.000}},
        {{-27.090, 0.000, 67.830}, {0.000, -2.167, 0.000}},
        {{-26.779, 0.000, 75.762}, {-0.000, -2.167, 0.000}},
        {{43.040, 0.000, 77.959}, {0.000, -2.167, 0.000}},
        {{26.415, 0.000, 77.990}, {0.000, 2.473, 0.000}},
        {{29.941, 0.000, 1.047}, {0.000, -0.245, 0.000}},
    };
    for (auto p : objPoints)
    {
        Model3D* obj = new Model3D("Skeleton_lean.gltf", p[0], p[1], VEC3_ONE, &sMat);
        decor->adopt(obj);
    }
    objPoints.clear();
    // special case
    Model3D* skeleton = new Model3D("Skeleton_lean.gltf", {-26.621, 0.000, 73.940}, {0.000, -1.565, 0.000}, {-1.000, 1.000, 1.000}, &sMat);
    decor->adopt(skeleton);

    // Torches
    objPoints = {
        {{-5.474, 3.733, 1.001}, {0.000, 0.000, -0.000}},
        {{5.357, 3.733, 1.001}, {0.000, 0.000, -0.000}},
        {{0.148, 3.733, 11.027}, {0.000, 0.000, -0.000}},
        {{-39.362, 3.733, 11.027}, {0.000, 0.000, -0.000}},
        {{25.257, 3.733, 1.001}, {0.000, 0.000, -0.000}},
        {{35.110, 3.733, 1.001}, {0.000, 0.000, -0.000}},
        {{-10.081, 3.733, 20.995}, {0.000, 0.000, -0.000}},
        {{-30.174, 3.733, 20.995}, {0.000, 0.000, -0.000}},
        {{-10.660, 3.733, 50.950}, {0.000, 0.000, -0.000}},
        {{-25.113, 3.733, 58.292}, {0.000, -3.142, 0.000}},
        {{-0.460, 3.733, 58.348}, {0.000, -3.142, 0.000}},
        {{10.139, 3.733, 78.339}, {0.000, -3.142, 0.000}}
    };
    for (auto p : objPoints)
    {
        Model3D* obj = new Model3D("Torch.gltf", p[0], p[1], VEC3_ONE, &sMat);
        decor->adopt(obj);
    }
    objPoints.clear();

    // lit torches
    objPoints = {
        {{4.600, 3.733, -5.000}, {0.000, -1.571, 0.000}},
        {{-4.600, 3.733, -5.000}, {0.000, 1.571, 0.000}},
        {{-4.600, 3.733, 85.010}, {0.000, 1.571, 0.000}},
        {{4.600, 3.733, 84.900}, {0.000, 4.712, -0.000}}
    };
    for (auto p : objPoints)
    {
        Model3D* torch = new Model3D("Torch.gltf", p[0], p[1], VEC3_ONE, &metalMat);
        constexpr float flameScale = 0.5f;
        constexpr float flameOffset = 0.7139f;
        for (int i = 0; i < 3; i++)
        {
            Model3D* flame = new Model3D("Unit Plane.gltf", VEC3_ZERO, {1.571, i * 1.571, 0}, VEC3_ONE * flameScale, &flame3);
            torch->adopt(flame);
            flame->setLocalPosition({0, flameOffset, 0});
        }
        PointLight* torchLight = new PointLight(VEC3_ZERO, 5, {1, 0.541, 0}, 8, 0.5);
        torch->adopt(torchLight);
        torchLight->setLocalPosition({0, flameOffset, 0});

        decor->adopt(torch);
    }
    objPoints.clear();

    // Lights
    AmbientLight *ambientLight = new AmbientLight({0.08f, 0.14f, 0.20f},{0.035f, 0.04f, 0.045f}, {0.0f, 1.0f, 0.0f});
    ambientLight->name = "AmbientLight";
    root->adopt(ambientLight);

    DirectionalLight *directionalLight = new DirectionalLight(0.5,glm::vec3(1.0f, 0.95f, 0.8f),glm::normalize(glm::vec3(0.8f, 0.25f, 0.4f)));
    directionalLight->name = "DirectionalLight";
    root->adopt(directionalLight);

    // FPS
    root->adopt(new FPSTextUpdater());

    // TutorialText
    Text2D *cameraTutorial = new Text2D("Press left mouse to cast spell", {-1, 1}, "SS", false, false, false, TAL_LEFT, TRH_LEFT, TRV_BOTTOM);
    root->adopt(cameraTutorial);

    return root;
}

Node* createMainMenu() {
    Node *root = new Node();
    root->name = "root";

    root->adopt(new FPSTextUpdater());

    root->adopt(new AudioController(new AudioNode("heavyRain.wav", 0.02f), true));

    // Models
    Node *models = new Node();
    models->name = "ModelContainer";
    root->adopt(models);

    Model3D* castle = new Model3D("Castle.gltf", {0, -35.0f, 35.0f}, {0, M_PI - 1, 0}, VEC3_ONE, TEX_MAT("Diffuse_palette.jpg"));
    models->adopt(castle);
    Model3D* door = new Model3D("Door.gltf", VEC3_ZERO, VEC3_ZERO, VEC3_ONE, wood_metalMat);
    castle->adopt(door);

    Model3D *plane = new Model3D("Unit Plane.gltf", VEC3_ZERO, VEC3_ZERO, {100, 1, 100}, pathMat);
    castle->adopt(plane);
    Model3D *plane2 = new Model3D("Unit Plane.gltf", {-70.0f, 0, 0}, VEC3_ZERO, {40, 1, 100}, lowGrassMat);
    castle->adopt(plane2);
    Model3D *plane3 = new Model3D("Unit Plane.gltf", {0.0f, 0, -75.0f}, VEC3_ZERO, {180, 1, 50}, lowGrassMat);
    castle->adopt(plane3);

    Model3D *rootTree = new Model3D("Spooky Tree.gltf", {-10.0f, 0.0f, 20.0f}, {0.0f, M_PI, 0.0f}, VEC3_ONE, &treeMat);
    castle->adopt(rootTree);

    std::list<std::array<glm::vec3, 3>> treesVec = {
        {glm::vec3(0.25f, 0.0f,10.0f), {0.0f, 5.12f, 0.0f}, VEC3_ONE * 1.5f},
        {glm::vec3(-1.0f, 0.0f,-11.0f), {0.0f, 4.77f, 0.0f}, VEC3_ONE * 1.1f},
        {glm::vec3(3.0f, 0.0f,-23.0f), {0.0f, 0.45f, 0.0f}, VEC3_ONE * 0.9f},
        {glm::vec3(5.0f, 0.0f,-16.0f), {0.0f, 0.2f, 0.0f}, VEC3_ONE * 1.15f},
        {glm::vec3(15.09f, 0.0f,5.2f), {0.0f, 4.33f, 0.0f}, VEC3_ONE * 2.47f},
        {glm::vec3(6.99f, 0.0f,-2.50f), {0.0f, 6.05f, 0.0f}, VEC3_ONE * 1.99f},
        {glm::vec3(11.01f, 0.0f,-12.41f), {0.0f, 2.48f, 0.0f}, VEC3_ONE * 1.68f},
        {glm::vec3(13.55f, 0.0f,-21.05f), {0.0f, 2.14f, 0.0f}, VEC3_ONE * 1.05f},
        {glm::vec3(24.0f, 0.0f,-10.0f), {0.0f, 6.05f, 0.0f}, VEC3_ONE * 2.5f},
        {glm::vec3(15.0f, 0.0f,22.0f), {0.0f, 3.05f, 0.0f}, VEC3_ONE * 3.3f},
        {glm::vec3(29.0f, 0.0f,55.0f), {0.0f, 1.05f, 0.0f}, VEC3_ONE * 4.5f},
        
        {glm::vec3(40.0f, 0.0f,0.0f), {0.0f, 5.86f, 0.0f}, VEC3_ONE * 3.2f},
        {glm::vec3(56.0f, 0.0f,-17.8f), {0.0f, 2.0f, 0.0f}, VEC3_ONE * 3.5f},
        {glm::vec3(33.2f, 0.0f,-24.3f), {0.0f, 0.39f, 0.0f}, VEC3_ONE * 2.43f},
        {glm::vec3(44.7f, 0.0f,20.0f), {0.0f, 2.79f, 0.0f}, VEC3_ONE * 2.7f},
        {glm::vec3(46.9f, 0.0f,45.0f), {0.0f, 2.62f, 0.0f}, VEC3_ONE * 2.2f},
        {glm::vec3(22.0f, 0.0f,87.0f), {0.0f, 1.77f, 0.0f}, VEC3_ONE * 2.8f},
        {glm::vec3(0.0f, 0.0f,102.0f), {0.0f, 1.49f, 0.0f}, VEC3_ONE * 3.2f},

        {glm::vec3(-55.0f, 10.0f,20.0f), {1.0f, 1.0f, 0.0f}, VEC3_ONE * 3.5f},
        {glm::vec3(-35.0f, 15.0f,-15.0f), {0.5f, 3.0f, 1.0f}, VEC3_ONE * 3.5f},

        {glm::vec3(-30.0f, 0.0f,12.0f), {0.0f, 1.41f, 0.0f}, VEC3_ONE * 1.5f},
        {glm::vec3(-22.0f, 0.0f,37.0f), {0.0f, 4.52f, 0.0f}, VEC3_ONE * 1.9f},
        {glm::vec3(-37.0f, 0.0f,66.0f), {0.0f, 5.56f, 0.0f}, VEC3_ONE * 2.4f},
        {glm::vec3(-34.0f, 0.0f,25.0f), {0.0f, 1.05f, 0.0f}, VEC3_ONE * 2.1f},
        {glm::vec3(-43.0f, 0.0f,20.0f), {0.0f, 0.01f, 0.0f}, VEC3_ONE * 2.1f},
        {glm::vec3(-32.0f, 0.0f,0.0f), {0.0f, 3.68f, 0.0f}, VEC3_ONE * 2.6f},
        {glm::vec3(-38.0f, 0.0f,8.0f), {0.0f, 2.18, 0.0f}, VEC3_ONE * 1.3f},
    };
    for (auto t : treesVec) {
        Model3D *newTree = new Model3D("Spooky Tree.gltf", t[0], t[1], t[2], &treeMat);
        rootTree->adopt(newTree);
    }

    LambertTexMaterial* mat = new LambertTexMaterial(VEC3_ONE, {1, 1, 1, 100}, "mage.png");
    Model3D* mage = new Model3D("Mage.gltf", {0.0f, 0.0f, 10.0f}, {0.0f, M_PI, 0.0f}, VEC3_ONE, mat);
    castle->adopt(mage);

    PointLight *mageLight = new PointLight({0.0f, 2.5f, 0.5f}, 1.0f, {1.0f, 0.0f, 1.0f}, 4, 2);
    mage->adopt(mageLight);

    PerspectiveCamera *camera = new PerspectiveCamera(0.1f, 200, glm::radians(90.0f), 4.0f/3.0f, true);
    camera->name = "PerspectiveCamera";
    camera->lookAt({castle->getGlobalPosition()});
    root->adopt(camera);

    Particles* rain = new Particles({0.0f,10.0f,0.0f}, {0.0f, 0.0f, 0.0f}, "", &rainMat);
    rain->setMaxParticles(20);
    rain->setBounds({10.0f, 10.0f, 10.0f});
    rain->setMaxLifeTime({0.5f, 1.0f});
    rain->setLinearVelocity({0.0f, -15.0f, 0.0f});
    rain->setTarget(camera);
    models->adopt(rain);

    // Lights
    AmbientLight *ambientLight = new AmbientLight({0.08f, 0.14f, 0.20f},{0.035f, 0.04f, 0.045f}, {0.0f, 1.0f, 0.0f});
    ambientLight->name = "AmbientLight";
    root->adopt(ambientLight);

    DirectionalLight *directionalLight = new DirectionalLight(0.5,glm::vec3(1.0f, 0.95f, 0.8f),glm::normalize(glm::vec3(0.8f, 0.25f, 0.4f)));
    directionalLight->name = "DirectionalLight";
    root->adopt(directionalLight);

    return root;
}

Node* createEndMenu() {
    Node *root = new Node();
    root->name = "root";

    SpinningMageUpdate *speen = new SpinningMageUpdate();
    speen->globalTranslate({0.0f, 0.0f, 5.0f});
    root->adopt(speen);

    LambertTexMaterial *mat = new LambertTexMaterial(VEC3_ONE, {1, 1, 1, 100}, "mage.png");
    Model3D *mage = new Model3D("Mage.gltf", {0.0f, 0.0f, 0.0f}, {0.0f, M_PI, 0.0f}, VEC3_ONE, mat);
    root->adopt(mage);
    speen->adopt(mage);

    Model3D *background = new Model3D("Unit Plane.gltf", {0, 0, 10.0f}, {std::numbers::pi / 2, 0, std::numbers::pi}, {30, 1, 30}, &rMat);
    background->localRotateX(0.5f);
    root->adopt(background);

    PerspectiveCamera *camera = new PerspectiveCamera(0.1f, 400, glm::radians(90.0f), 4.0f/3.0f, true);
    camera->name = "PerspectiveCamera";
    camera->localTranslate({0, 5.0f, 0});
    camera->localRotateY(M_PI);
    camera->localRotateX(-0.5f);
    root->adopt(camera);

    // Lights
    AmbientLight *ambientLight = new AmbientLight({0.08f, 0.14f, 0.20f},{0.035f, 0.04f, 0.045f}, {0.0f, 1.0f, 0.0f});
    ambientLight->name = "AmbientLight";
    root->adopt(ambientLight);

    DirectionalLight *directionalLight = new DirectionalLight(0.5,glm::vec3(1.0f, 0.95f, 0.8f),glm::normalize(glm::vec3(0.8f, 0.25f, 0.4f)));
    directionalLight->name = "DirectionalLight";
    root->adopt(directionalLight);

    return root;
}

int main() {

    Engine engine;

    // Globals
    Engine::setGlobalVariable("SonarMaterialReference", &sMat);

    // Scenes
    Engine::setGlobalVariable("Scene1", createScene1());
    Engine::setGlobalVariable("Scene2", createScene2());
    Engine::setGlobalVariable("Unit", createUnitScene());
    Engine::setGlobalVariable("Forest", createForestScene());
    Engine::setGlobalVariable("Dark", createLabyrinthScene());
    Engine::setGlobalVariable("MainMenu", createMainMenu());
    Engine::setGlobalVariable("EndMenu", createEndMenu());

    Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("MainMenu")));

    try {
        engine.run(false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Free the scenes
    freeNodeTree(std::any_cast<Node*>(Engine::getGlobalVariable("MainMenu")));
    freeNodeTree(std::any_cast<Node*>(Engine::getGlobalVariable("Scene1")));
    freeNodeTree(std::any_cast<Node*>(Engine::getGlobalVariable("Scene2")));
    freeNodeTree(std::any_cast<Node*>(Engine::getGlobalVariable("Unit")));
    freeNodeTree(std::any_cast<Node*>(Engine::getGlobalVariable("Forest")));
    freeNodeTree(std::any_cast<Node*>(Engine::getGlobalVariable("Dark")));
    freeNodeTree(std::any_cast<Node*>(Engine::getGlobalVariable("EndMenu")));

    return EXIT_SUCCESS;
}
