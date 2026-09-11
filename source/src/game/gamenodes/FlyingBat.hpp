#include "UpdateNode3D.hpp"
#define MAX_WING_ANGLE 30.0f

class FlyingBat : public UpdateNode3D {
    Material* mat;

    Model3D* body;
    Model3D *wing1, *wing2;

    float wingSpeed, movSpeed;
    float radius;
    public:
    FlyingBat(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, float wingSpeed, float movSpeed, float radius, Material* mat) {
        setGlobalPosition(position);
        setGlobalRotation(rotation);
        setGlobalScale(scale);
        this->mat = mat;
        this->wingSpeed = wingSpeed;
        this->movSpeed = movSpeed;
        this->radius = radius;
    }

    Node* createBody() {
        body = new Model3D("Bat_Body.gltf", {0,0,radius}, {0,0,glm::radians(-15.0f)}, {1,1,1}, mat);

        wing1 = new Model3D("Bat_Wing.gltf", {0,0,0}, {0, 0,0}, {0.15,0.15,0.15}, mat);
        body->adopt(wing1);
        wing2 = new Model3D("Bat_Wing2.gltf", {0,0,0}, {0, 0,0}, {0.15,0.15,0.15}, mat);
        body->adopt(wing2);

        return body;
    }

    void onEnter() override {

    }


    void update() override {
        float t = Engine::getCurrentTime();

        float angle = glm::radians(MAX_WING_ANGLE) * sin(wingSpeed*t);
        wing1->setLocalRotation({0,angle,0});
        wing2->setLocalRotation({0,-angle,0});

        /*
        glm::vec2 pos = RADIUS * glm::vec2(cos(movSpeed*t), sin(movSpeed*t));
        body->setGlobalPosition({pos.x,body->getGlobalPosition().y,pos.y});*/

        setLocalRotation({0,movSpeed*t,0});
    }

    void onExit() override {

    }
};