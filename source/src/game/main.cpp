#include "Engine.hpp"
#include <iostream>
#include <ostream>
#include "OrthoCamera.hpp"
#include "PerspectiveCamera.hpp"
#include "gamenodes/CustomCameraUpdate.hpp"

int main() {

    Engine engine;

    Node *root = new Node();
    CustomCameraUpdate *cameraContainer = new CustomCameraUpdate();
    PerspectiveCamera *camera1 = new PerspectiveCamera(0.01, 100, glm::radians(90.0f), 4.0f/3.0f);
    OrthoCamera *camera2 = new OrthoCamera(-4, 4, -2, 2, 0.01, 100);
    root->adopt(cameraContainer);
    cameraContainer->adopt(camera1);
    cameraContainer->adopt(camera2);

    engine.setScene(root);
    engine.setMainCamera(camera1);


    try {
        engine.run(false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
