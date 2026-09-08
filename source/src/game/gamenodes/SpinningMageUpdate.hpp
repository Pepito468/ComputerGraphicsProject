#include "Engine.hpp"
#include "UpdateNode3D.hpp"

class SpinningMageUpdate : public UpdateNode3D {
    virtual void update() override {
        this->localRotateY(Engine::MainEngine->getDeltaTime() * 5);
    }
};
