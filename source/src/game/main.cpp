#include <iostream>
#include <ostream>

#include "Engine.hpp"

#include "PerspectiveCamera.hpp"
#include "OrthoCamera.hpp"

#include "Model3D.hpp"

#include "PointLight.hpp"
#include "DirectionalLight.hpp"
#include "AmbientLight.hpp"

#include "gamenodes/CustomCameraUpdate.hpp"
#include "gamenodes/MovingPlanetUpdate.hpp"
#include "gamenodes/MovingPlanetChildUpdate.hpp"

int main() {

    Engine engine;

    Node *root = new Node();
    root->name = "root";
    CustomCameraUpdate *cameraContainer = new CustomCameraUpdate();
    cameraContainer->name = "CameraContainer";
    PerspectiveCamera *camera1 = new PerspectiveCamera(0.01, 100, glm::radians(90.0f), 4.0f/3.0f);
    camera1->name = "PerspectiveCamera";
    OrthoCamera *camera2 = new OrthoCamera(-4, 4, -2, 2, 0.01, 200);
    camera2->name = "OrthoCamera";
    root->adopt(cameraContainer);
    cameraContainer->adopt(camera1);
    cameraContainer->adopt(camera2);

    engine.setSceneRoot(root);
    engine.setMainCamera(camera1);

    // Models
    LambertMaterial mat3 = {glm::vec3(1.0f, 0.0f, 0.0f), {1.0f,1.0f,1.0f,100.0f}};
    ToonMaterial mat2 = {glm::vec3(0.9f, 0.45f, 0.9f), {1.0f,1.0f,1.0f,100.0f}, 0.3f, 1.0f, 0.3f, 0.95f, 1.0f, 0.0f};
    LambertTexMaterial mat1 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "rock.png"};
    //LambertTexMaterial mat2 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "VChecker.png"};
    WaterMaterial mat4 = {glm::vec3(1.0f, 1.0f, 1.0f), {1.0f,1.0f,1.0f,100.0f}, "water.png"};

    Node *models = new Node();
    models->name = "ModelContainer";
    root->adopt(models);

    Model3D *statue = new Model3D("Statue.gltf", {1, -0.5, 0}, {0, 0, 0}, {4, 4, 4}, &mat1);
    statue->name = "Statue";
    models->adopt(statue);

    Model3D *plane = new Model3D("Water.gltf", {0, 0.1, 0}, {0, 0, 0}, {10, 10, 10}, &mat4);
    plane->name = "Plane";
    models->adopt(plane);

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


    try {
        engine.run(false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
