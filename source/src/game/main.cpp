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
    PerspectiveCamera *camera = new PerspectiveCamera(0.01, 100, glm::radians(90.0f), 4.0f/3.0f);
    // OrthoCamera *camera = new OrthoCamera(-4, 4, -5, 5, 0.01, 100);
    root->adopt(cameraContainer);
    cameraContainer->adopt(camera);

    engine.setScene(root);
    engine.setMainCamera(camera);


    try {
        engine.run(false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
