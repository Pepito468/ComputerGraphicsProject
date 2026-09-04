#ifndef ENGINE_PARTICLES_HPP
#define ENGINE_PARTICLES_HPP
#include "Engine.hpp"
#include "Model3D.hpp"
#include "UpdateNode3D.hpp"

#include <ctime>
#include <cstdlib>

class Particles : public UpdateNode3D {

    class SingleParticle : public Model3D {
        float timer;
        Particles* particles;

        float maxLifeTime;

        public:
        SingleParticle(std::string modelPath, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale, Material* material, Particles* particles) :
            Model3D(modelPath, position, rotation, scale, material) {
            this->particles = particles;

            maxLifeTime = particles->getRand(particles->maxLifeTime.x, particles->maxLifeTime.y);
        }

        void update() {
            if (isVisible) {
                timer += Engine::getDeltaTime();

                //apply linear velocity
                globalTranslate(particles->linearVelocity*Engine::getDeltaTime());

                //check lifeTime
                if (timer >= maxLifeTime) {
                    particles->resetPosition(this);
                    timer = 0.0f;
                }
            }

        }
    };

    int maxParticles;
    glm::vec2 maxLifeTime;

    glm::vec3 boxBounds;
    glm::vec3 linearVelocity;

    std::string textureName;
    Material* material;

    float timer;

    Node3D* target = nullptr;

    public:

    Particles(const glm::vec3 position, const glm::vec3 rotation, const std::string textureName, Material* material) :
        UpdateNode3D(position, rotation, glm::vec3(1.0f, 1.0f, 1.0f))
    {
        this->textureName = textureName;
        this->material = material;

        srand(time(nullptr));
    }

    void setTarget(Node3D* target) {
        this->target = target;
    }

    void onEnter() override {
        for (size_t i = 0; i < 100; i++) {
            spawnParticle();
        }
    }

    //float currentPosInCircle = 0.0f;
    void update() override {
        //Update all child
        for (Node* child : this->children) {
            SingleParticle* p = dynamic_cast<SingleParticle*>(child);

            p->update();
        }
    }

    void setBounds(glm::vec3 boxBounds) {
        this->boxBounds = boxBounds;
    }

    void setLinearVelocity(glm::vec3 linearVelocity) {
        this->linearVelocity = linearVelocity;
    }

    void setMaxLifeTime(glm::vec2 maxLifeTime) {
        this->maxLifeTime = maxLifeTime;
    }

    void setMaxParticles(int maxParticles) {
        this->maxParticles = maxParticles;
    }

    private:

    //from: https://lucidar.me/en/c-class/lesson-08-08-random-numbers-in-c/
    float getRand(float min, float max) {
        return min + (float)rand() / ((float)RAND_MAX/(max-min));
    }

    void resetPosition(SingleParticle* sp) {
        glm::vec3 spawnPos = target != nullptr ? target->getGlobalPosition() + getGlobalPosition() : getGlobalPosition();

        sp->setGlobalPosition(spawnPos);
        sp->globalTranslate({getRand(-boxBounds.x, boxBounds.x),getRand(-boxBounds.y, boxBounds.y),getRand(-boxBounds.z, boxBounds.z)});
    }

    void spawnParticle() {
        glm::vec3 spawnPos = target != nullptr ? target->getGlobalPosition() + getGlobalPosition() : getGlobalPosition();

        SingleParticle* p = new SingleParticle("Unit Cube.gltf", spawnPos, {0.0f, 0.0f, 0.0f}, {0.05f, 0.2f, 0.05f}, material, this);
        p->globalTranslate({getRand(-boxBounds.x, boxBounds.x), getRand(-boxBounds.y, boxBounds.y),getRand(-boxBounds.z, boxBounds.z)});

        this->adopt(p);
    }

};

#endif
