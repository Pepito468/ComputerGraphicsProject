#include "Engine.hpp"
#include "UpdateNode3D.hpp"

class MovingPlanetUpdate : public UpdateNode3D {

    float currentPosInCircle = 0.0f;

    virtual void onEnter() override {
        // this->globalTranslate({0, 2, 0});
    }

    virtual void update() override {
        currentPosInCircle += Engine::MainEngine->getDeltaTime() * 0.5;

        this->setGlobalPosition(glm::vec3(sin(currentPosInCircle)*10, this->getGlobalPosition().y, cos(currentPosInCircle)*10));
    }

};
