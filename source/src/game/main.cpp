#include <any>
#include <iostream>
#include <ostream>

#include "Engine.hpp"

#include "Material.hpp"
#include "PerspectiveCamera.hpp"
#include "OrthoCamera.hpp"

#include "Model3D.hpp"

#include "PointLight.hpp"
#include "DirectionalLight.hpp"
#include "AmbientLight.hpp"
#include "ColliderLib.hpp"

#include "audio/AudioNode.hpp"
#include "audio/AudioNode3D.hpp"

#include "common.h"
#include "gamenodes/CustomCameraUpdate.hpp"
#include "gamenodes/MovingPlanetUpdate.hpp"
#include "gamenodes/MovingPlanetChildUpdate.hpp"
#include "gamenodes/PlayerNode.hpp"

// LambertMaterial mat3 = {glm::vec3(1.0f, 0.0f, 0.0f), {1.0f,1.0f,1.0f,100.0f}};
// ToonMaterial mat2 = {glm::vec3(0.9f, 0.45f, 0.9f), {1.0f,1.0f,1.0f,100.0f}, 0.3f, 1.0f, 0.3f, 0.95f, 1.0f, 0.0f};
// LambertTexMaterial mat1 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "rock.png"};
// //LambertTexMaterial mat2 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "VChecker.png"};
// WaterMaterial mat4 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "water.png"};

LambertMaterial mat3 = {glm::vec3(1.0f, 0.0f, 0.0f), {1.0f,1.0f,1.0f,100.0f}};
ToonMaterial mat2 = {glm::vec3(0.9f, 0.45f, 0.9f), {1.0f,1.0f,1.0f,100.0f}, 0.3f, 1.0f, 0.3f, 0.95f, 1.0f, 0.0f};
LambertTexMaterial mat1 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "rock.png"};
//LambertTexMaterial mat2 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "VChecker.png"};
WaterMaterial mat4 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "water.png"};
FireMaterial flame1 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "smallFlame.png"};
FireMaterial flame2 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "bigFlame.png"};
FireMaterial flame3 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "fire.png"};
MagicCirleMaterial mat5 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "magicCircle.png"};

CookTorranceAnimMaterial animMat = {glm::vec3(1.0f, 0.0f, 0.0f), {1.0f,1.0f,1.0f,100.0f}, "rock.png"};

RainbowMaterial rMat = {0.2, 1, 1, 0.3};
SonarMaterial sMat = {10, 4, 60};

void freeNodeTree(Node *node) {
    for (Node *child : node->children)
        freeNodeTree(child);
    delete node;
}

Node* createScene1() {

    Node *root = new Node();
    root->name = "root";
    CapsuleCollider* playerCollider = new CapsuleCollider();
    playerCollider->name = "PlayerCollider";
    playerCollider->layer = PLAYER;
    playerCollider->collidesWith = ENVIRONMENT;
    root->adopt(playerCollider);
    PlayerNode* player = new PlayerNode();
    player->name = "Player";
    playerCollider->adopt(player);
    PerspectiveCamera *camera = new PerspectiveCamera(0.01, 100, glm::radians(90.0f), 4.0f/3.0f, true);
    camera->name = "PerspectiveCamera";
    player->adopt(camera);
    playerCollider->globalTranslate({-2, 5, 0});
    player->localTranslate({0, 1.25f, 0});

    // Audio
    AudioNode *quack = new AudioNode("quack.mp3", 0.1f);
    quack->name = "quack";
    player->adopt(quack);


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
    Text2D *text = new Text2D("", {-1, -1}, "SS", false, true, true, TAL_LEFT, TRH_LEFT, TRV_TOP);
    root->adopt(text);
    player->inputtxt = text;

    // Models
    Node *models = new Node();
    models->name = "ModelContainer";
    root->adopt(models);

    Model3D *statue = new Model3D("Statue.gltf", {1, -0.5, 0}, {0, 0, 0}, {4, 4, 4}, &mat1);
    statue->name = "Statue";
    models->adopt(statue);

    Model3D *statuer = new Model3D("Statue.gltf", {-1, -0.5, 0}, {0, 0, 0}, {4, 4, 4}, &rMat);
    statuer->name = "Statue";
    models->adopt(statuer);

    // Sonar plane
    Model3D *sonar = new Model3D("Water.gltf", {-3, 3, -10}, {std::numbers::pi/2, 0, 0}, {10, 10, 10}, &sMat);
    sonar->name = "Sonar";
    player->sonarMat = &sMat;
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

    Model3D *magicCircle = new Model3D("Water.gltf", {0, 0.5, 0}, {0, 0, 0}, {2, 2, 2}, &mat5);
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
    AudioNode3D *movingAudio = new AudioNode3D("rocketJumpWaltz.mp3", 1.0f, 50.0f, 1.0f, INVERSE, 0.1f);
    movingAudio->name = "RocketJumWaltz";
    planet->adopt(movingAudio);
    player->music = movingAudio;

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

Node* createDarkScene() {

    Node *root = new Node();
    root->name = "root";

    // Player
    CapsuleCollider *playerCollider = new CapsuleCollider();
    playerCollider->name = "PlayerCollider";
    playerCollider->layer = PLAYER;
    playerCollider->collidesWith = ENVIRONMENT;
    root->adopt(playerCollider);
    PlayerNode* player = new PlayerNode();
    player->name = "Player";
    playerCollider->adopt(player);
    PerspectiveCamera *camera = new PerspectiveCamera(0.01, 100, glm::radians(90.0f), 4.0f/3.0f, true);
    camera->name = "PerspectiveCamera";
    player->adopt(camera);
    player->localTranslate({0, 1.25f, 0});

    // Assign Sonar
    player->sonarMat = &sMat;

    playerCollider->globalTranslate({0, 20, 0});


    Node *staticObjects = new Node();
    staticObjects->name = "StaticObjectsContainer";
    root->adopt(staticObjects);

#define LABSIZE 12
    float cellSize = 8;
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
    BoxCollider* floorCollider = new BoxCollider(5.0f, 0.05f, 5.0f);
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
                case '.':
                    break;
                case 'W':
                    for (int k = 0; k < 4; k++) {
                        const glm::vec3 pos = glm::vec3(i*cellSize, cellSize/2, j*cellSize) + offset[k];
                        Model3D *wall = new Model3D("Water.gltf", pos, rotations[k], {cellSize/2, 1, cellSize/2}, &sMat);
                        wall->name = std::format("Wall {} {} {}", i, j, k);
                        staticObjects->adopt(wall);

                        // Colliders tank the FPS and idk where they are when spawned
                        BoxCollider* wallCollider = new BoxCollider(0.5f, 0.5f, 0.5f);
                        wall->name = std::format("WallColl {} {} {}", i, j, k);
                        wallCollider->movementStatus = STATIC;
                        wallCollider->layer = ENVIRONMENT;
                        // wall->adopt(wallCollider);
                    }
                    break;
                case 'S':
                    // Maybe add a room before the maze? (and after?)
                    playerCollider->setGlobalPosition({i*cellSize, 30, j*cellSize});
                    break;
                case 'E':
                    // Do something
                    break;
                default:
                    error("What?");
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
    Text2D *text = new Text2D("", {-1, -1}, "SS", false, true, true, TAL_LEFT, TRH_LEFT, TRV_TOP);
    root->adopt(text);
    player->inputtxt = text;

    return root;
}

int main() {

    Engine engine;

    Engine::setGlobalVariable("Scene1", createScene1());
    Engine::setGlobalVariable("Scene2", createScene2());
    Engine::setGlobalVariable("SceneDark", createDarkScene());

    Engine::requestSceneChange(std::any_cast<Node*>(Engine::getGlobalVariable("SceneDark")));

    try {
        engine.run(false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Free the scenes
    freeNodeTree(std::any_cast<Node*>(Engine::getGlobalVariable("Scene1")));
    freeNodeTree(std::any_cast<Node*>(Engine::getGlobalVariable("Scene2")));
    freeNodeTree(std::any_cast<Node*>(Engine::getGlobalVariable("SceneDark")));

    return EXIT_SUCCESS;
}
