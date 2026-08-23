#include "Engine.hpp"
#include <iostream>
#include <ostream>
#include "Light.hpp"
#include "Model3D.hpp"
#include "OrthoCamera.hpp"
#include "PerspectiveCamera.hpp"
#include "gamenodes/CustomCameraUpdate.hpp"
#include "glm/geometric.hpp"

int main() {

    Engine engine;

    Node *root = new Node();
    root->name = "root";
    CustomCameraUpdate *cameraContainer = new CustomCameraUpdate();
    cameraContainer->name = "CameraContainer";
    PerspectiveCamera *camera1 = new PerspectiveCamera(0.01, 100, glm::radians(90.0f), 4.0f/3.0f);
    camera1->name = "PerspectiveCamera";
    OrthoCamera *camera2 = new OrthoCamera(-4, 4, -2, 2, 0.01, 100);
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

    Node *models = new Node();
    models->name = "ModelContainer";
    root->adopt(models);

    Model3D *statue = new Model3D("Statue.gltf", {1, 0, 0}, {0, 0, 0}, {1, 1, 1}, &mat1);
    statue->name = "Statue";
    models->adopt(statue);

    Model3D *plane = new Model3D("Plane.gltf", {0, 0, 0}, {0, 0, 0}, {10, 10, 10}, &mat1);
    plane->name = "Plane";
    models->adopt(plane);

    // Lights
    AmbientLight *ambientLight = new AmbientLight({0.01, 0.01, 0.01}, {0.2, 0.15, 0.1}, {0, 1, 0});
    ambientLight->name = "AmbientLight";
    root->adopt(ambientLight);

    DirectionalLight *directionalLight = new DirectionalLight(0.01, {1, 0.95, 0.8}, glm::normalize(glm::vec3(0.4, 0.8, 0.4)));
    directionalLight->name = "DirectionalLight";
    // root->adopt(directionalLight);

    PointLight *pointLight = new PointLight({1, 1, 0}, 1, {1, 0, 0}, 20, 2);
    pointLight->name = "PointLight";
    root->adopt(pointLight);

    try {
        engine.run(false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
