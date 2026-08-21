#include "UpdateNode3D.hpp"
#include "Engine.hpp"

class CustomCameraUpdate : public UpdateNode3D {

    void onEnter() override {
        // Position over the plane
        this->globalTranslate({0, 2, 0});
    }

    void update() override {
        // update the camera position and direction with the inputs
        glm::vec3 xdir = glm::normalize(glm::vec3(this->getXAxis().x, 0, this->getXAxis().z));
        glm::vec3 zdir = glm::normalize(glm::vec3(this->getZAxis().x, 0, this->getZAxis().z));

        if (engineGlobals.inputTranslation.x == 1)
            this->globalTranslate(xdir * engineGlobals.deltaTime);
        else if (engineGlobals.inputTranslation.x == -1)
            this->globalTranslate(xdir * engineGlobals.deltaTime * -1.0f);
        // Z axis is positive on the back
        if (engineGlobals.inputTranslation.z == 1)
            this->globalTranslate(zdir * engineGlobals.deltaTime);
        else if (engineGlobals.inputTranslation.z == -1)
            this->globalTranslate(zdir * engineGlobals.deltaTime * -1.0f);

        // TODO: find method to query input (window from engine is needed: API from Engine or Engine itself passed to the node?)

        this->globalRotateX(-engineGlobals.inputRotation.x * engineGlobals.deltaTime);
        this->globalRotateY(-engineGlobals.inputRotation.y * engineGlobals.deltaTime);

    }

};
