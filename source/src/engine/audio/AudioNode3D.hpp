#ifndef ENGINE_AUDIONODE3D_H
#define ENGINE_AUDIONODE3D_H

#include "Node3D.hpp"

#include "AudioNode.hpp"

typedef enum {INVERSE, LINEAR, EXPONENTIAL} ATTENUTATION_MODE;

class AudioNode3D : public Node3D, public AudioNode {

    private:

        ATTENUTATION_MODE attenuationMode = INVERSE;

        float maxDistance = 10.0f;

        float minDistance = 1.0f;

        float rolloff = 1.0f;


    public:

        AudioNode3D() {}

        AudioNode3D(const std::string filename, const float minDistance, const float maxDistance, const float rolloff, ATTENUTATION_MODE attenuationMode, const float volume = 1.0f) :
        AudioNode(filename, volume) {
            this->rolloff = rolloff;
            this->minDistance = minDistance;
            this->maxDistance = maxDistance;
            this->attenuationMode = attenuationMode;
        }

        ~AudioNode3D() {
            this->stopSound();
        }

        virtual void playSound() override {
            // Checks
            if (!audioEngine)
                error("No audio Engine set up");
            if (this->isInitialized) {
                ma_sound_uninit(&this->sound);
                this->isInitialized = false;
            }

            // Try and initialize
            const ma_result res = ma_sound_init_from_file(this->audioEngine, (AUDIO_DIR + this->soundFileName).c_str(), 0, NULL, NULL, &sound);
            if (res != MA_SUCCESS) {
                warning(std::format("Failed to play audio for [{}] ", this->UUID));
                return;
            }

            this->isInitialized = true;


            // Enable 3D sound
            ma_sound_set_spatialization_enabled(&this->sound, MA_TRUE);
            switch (this->attenuationMode) {
                case INVERSE:
                    ma_sound_set_attenuation_model(&this->sound, ma_attenuation_model_inverse);
                    break;
                case LINEAR:
                    ma_sound_set_attenuation_model(&this->sound, ma_attenuation_model_linear);
                    break;
                case EXPONENTIAL:
                    ma_sound_set_attenuation_model(&this->sound, ma_attenuation_model_exponential);
                    break;
            }
            ma_sound_set_min_distance(&this->sound, this->minDistance);
            ma_sound_set_max_distance(&this->sound, this->maxDistance);
            ma_sound_set_rolloff(&this->sound, this->rolloff);

            // Play
            ma_sound_set_volume(&this->sound, this->volume);
            ma_sound_start(&this->sound);

            log(std::format("Playing [{}] from [{}]", this->soundFileName, this->UUID));
        }

    private:

        void onTransformUpdate() override {
            if (!this->isInitialized)
                return;

            glm::vec3 position = this->getGlobalPosition();
            ma_sound_set_position(&this->sound, position.x, position.y, position.z);
        }

};

#endif
