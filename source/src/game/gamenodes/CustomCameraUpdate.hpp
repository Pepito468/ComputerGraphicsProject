#include "PerspectiveCamera.hpp"
#include "OrthoCamera.hpp"
#include "UpdateNode3D.hpp"
#include "Engine.hpp"
#include "glm/trigonometric.hpp"

class CustomCameraUpdate : public UpdateNode3D {

    PerspectiveCamera *PCamera = NULL;
    OrthoCamera *OCamera = NULL;

    void onEnter() override {
        // Position over the plane
        this->globalTranslate({-2, 2, 0});
        this->globalRotateY(glm::radians(-90.0f));

        for (Node *child : this->children) {
            if (PerspectiveCamera *camera = dynamic_cast<PerspectiveCamera*>(child))
                this->PCamera = camera;
            else if (OrthoCamera* camera = dynamic_cast<OrthoCamera*>(child))
                this->OCamera = camera;
        }
    }

    void update() override {

        // Check for escape
        if(Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_ESCAPE)) {
            Engine::MainEngine->shutdown();
        }

        // Camera change
        // if (Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_P)) {
        //     PCamera->setMain();
        // }
        // else if (Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_O))
        //     OCamera->setMain();

        // FOV update (ortho does not have FOV)
        if (Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_F))
            this->PCamera->setFOV(PCamera->getFOV() + glm::radians(Engine::MainEngine->getDeltaTime() * 10));

        // update the camera position and direction with the inputs
        glm::vec3 xdir = glm::normalize(glm::vec3(this->getXAxis().x, 0, this->getXAxis().z));
        glm::vec3 zdir = glm::normalize(glm::vec3(this->getZAxis().x, 0, this->getZAxis().z));

        if (Engine::MainEngine->getInputTranslation().x == 1)
            this->globalTranslate(xdir * Engine::MainEngine->getDeltaTime());
        else if (Engine::MainEngine->getInputTranslation().x == -1)
            this->globalTranslate(xdir * Engine::MainEngine->getDeltaTime() * -1.0f);
        // Z axis is positive on the back
        if (Engine::MainEngine->getInputTranslation().z == 1)
            this->globalTranslate(zdir * Engine::MainEngine->getDeltaTime());
        else if (Engine::MainEngine->getInputTranslation().z == -1)
            this->globalTranslate(zdir * Engine::MainEngine->getDeltaTime() * -1.0f);

        this->globalRotateX(-Engine::MainEngine->getInputRotation().x * Engine::MainEngine->getDeltaTime());
        this->globalRotateY(-Engine::MainEngine->getInputRotation().y * Engine::MainEngine->getDeltaTime());

        // Vertical movement
        if (Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_SPACE))
            this->globalTranslate(VEC3_Y * Engine::MainEngine->getDeltaTime());
        else if (Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_V))
            this->globalTranslate(VEC3_Y * Engine::MainEngine->getDeltaTime() * -1.0f);

    }

};
