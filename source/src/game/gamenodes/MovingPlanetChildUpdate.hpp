#include "Engine.hpp"
#include "UpdateNode3D.hpp"

class MovingPlanetChildUpdate : public UpdateNode3D {

    virtual void update() override {

        this->localRotateX(Engine::MainEngine->getDeltaTime() * 1);
        this->localRotateY(Engine::MainEngine->getDeltaTime() * 2);
        this->localRotateZ(Engine::MainEngine->getDeltaTime() * 3);

    }

};
