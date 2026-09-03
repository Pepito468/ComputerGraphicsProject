#ifndef ENGINE_LIGHTNING_NODE
#define ENGINE_LIGHTNING_NODE
#include "AudioNode.hpp"
#include "Engine.hpp"
#include "Light.hpp"
#include "UpdateNode3D.hpp"
#include "Types.hpp"

class LightningNode : public UpdateNode3D
{
    Light* light = nullptr;
    AudioNode* sound = nullptr;

    /// Peak light intensity of the lightning flash
    BoundFloat maxIntensity = PositiveFloat(1.0f);
    /// How many seconds pass between flashes
    BoundFloat interval = PositiveFloat(4.0f);
    /// How many seconds each flash lasts
    BoundFloat duration = PositiveFloat(1.0f);

    float timer = 0.0f;
    bool isFlashing = false;

public:
    LightningNode(Light* light, AudioNode* sound, const float maxIntensity, const float interval, const float duration)
    {
        this->light = light;
        this->sound = sound;
        this->maxIntensity = maxIntensity;
        this->interval = interval;
        this->duration = duration;

        Node::adopt(light);
        Node::adopt(sound);

        this->light->isOn = false;
        this->light->radiance = 0.0f;
        this->sound->setLooping(false);
    }

    void update() override
    {
        if (!light || !sound)
        {
            warning("Lightning node has NULL elements", true);
            return;
        }

        timer += Engine::getDeltaTime();
        if (!isFlashing)
        {
            if (timer >= interval)
            {
                //Begin flash
                isFlashing = true;
                this->light->isOn = true;
                this->light->radiance = maxIntensity;
                this->sound->playSound();

                timer = 0.0f;
            }
        }
        else
        {
            if (timer >= duration)
            {
                //End flash
                isFlashing = false;
                this->light->isOn = false;
                this->light->radiance = 0.0f;
                this->sound->stopSound();

                timer = 0.0f;
            }
            else
                this->light->radiance = maxIntensity * std::pow(2, -10*(timer/duration));
        }
    }
};
#endif