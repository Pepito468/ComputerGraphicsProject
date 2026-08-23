#include "../../engine/cameras/OrthoCamera.hpp"
#include "../../engine/cameras/PerspectiveCamera.hpp"
#include "UpdateNode3D.hpp"
#include "Engine.hpp"
#include "glm/trigonometric.hpp"

class CustomCameraUpdate : public UpdateNode3D {

    PerspectiveCamera *PCamera = NULL;
    OrthoCamera *OCamera = NULL;

    void onEnter() override {
        // Position over the plane
        this->globalTranslate({0, 2, 0});

        for (Node *child : this->children) {
            if (PerspectiveCamera *camera = dynamic_cast<PerspectiveCamera*>(child))
                this->PCamera = camera;
            else if (OrthoCamera* camera = dynamic_cast<OrthoCamera*>(child))
                this->OCamera = camera;
        }
    }

    void update() override {

        // Camera change
        if (Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_P))
            Engine::MainEngine->setMainCamera(PCamera);
        else if (Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_O))
            Engine::MainEngine->setMainCamera(OCamera);

        // FOV update (ortho does not have FOV)
        if (Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_F))
            this->PCamera->fov = PCamera->fov + glm::radians(Engine::MainEngine->deltaTime * 10);

        // update the camera position and direction with the inputs
        glm::vec3 xdir = glm::normalize(glm::vec3(this->getXAxis().x, 0, this->getXAxis().z));
        glm::vec3 zdir = glm::normalize(glm::vec3(this->getZAxis().x, 0, this->getZAxis().z));

        if (Engine::MainEngine->inputTranslation.x == 1)
            this->globalTranslate(xdir * Engine::MainEngine->deltaTime);
        else if (Engine::MainEngine->inputTranslation.x == -1)
            this->globalTranslate(xdir * Engine::MainEngine->deltaTime * -1.0f);
        // Z axis is positive on the back
        if (Engine::MainEngine->inputTranslation.z == 1)
            this->globalTranslate(zdir * Engine::MainEngine->deltaTime);
        else if (Engine::MainEngine->inputTranslation.z == -1)
            this->globalTranslate(zdir * Engine::MainEngine->deltaTime * -1.0f);

        this->globalRotateX(-Engine::MainEngine->inputRotation.x * Engine::MainEngine->deltaTime);
        this->globalRotateY(-Engine::MainEngine->inputRotation.y * Engine::MainEngine->deltaTime);

        // Vertical movement
        if (Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_SPACE))
            this->globalTranslate(VEC3_Y * Engine::MainEngine->deltaTime);
        else if (Engine::MainEngine->isKeyBeingPressed(GLFW_KEY_V))
            this->globalTranslate(VEC3_Y * Engine::MainEngine->deltaTime * -1.0f);

    }

};
