#ifndef ENGINE_AUDIOCONTROLLER
#define ENGINE_AUDIOCONTROLLER
#include "AudioNode.hpp"
#include "Engine.hpp"
#include "UpdateNode3D.hpp"

class AudioController : public UpdateNode3D
{
    AudioNode* audio = nullptr;
    bool playOnStart = false;
    bool loop = true;
    bool isPlaying = false;

public:
    AudioController(AudioNode* audio, const bool playOnStart = false, const bool loop = true)
    {
        this->audio = audio;
        this->playOnStart = playOnStart;
        this->loop = loop;

        Node::adopt(audio);
    }

    void update() override
    {
        if (!audio)
        {
            warning("AudioController audio set to NULL", true);
            return;
        }

        if (playOnStart)
        {
            audio->playSound();
            audio->setLooping(loop);
            playOnStart = false;
            isPlaying = true;
        }

        if (Engine::isKeyBeingPressed(GLFW_KEY_R, true)) {
            if (!isPlaying)
                audio->playSound();
            else
                audio->stopSound();

            isPlaying = !isPlaying;
            audio->setLooping(loop && isPlaying);
        }
    }

    AudioNode* getAudio() const {return audio;}
};
#endif
