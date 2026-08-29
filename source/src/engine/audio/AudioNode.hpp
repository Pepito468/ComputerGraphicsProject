#ifndef ENGINE_AUDIONODE_H
#define ENGINE_AUDIONODE_H

#include "Node.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define AUDIO_DIR "assets/"

class AudioNode : public virtual Node {

    protected:
        /** Audio engine */
        ma_engine *audioEngine = nullptr;

        /** Miniaudio  */
        ma_sound sound;

        /** Flags if the audio is initialized */
        bool isInitialized = false;


        /** Audio file name */
        std::string soundFileName = "";

        /** Volume (from 0.0 to 1.0) */
        float volume = 1.0f;


    public:

        AudioNode() {}

        AudioNode(const std::string filename, const float volume = 1.0f) {
            this->soundFileName = filename;
            this->volume = volume;
        }

        ~AudioNode() {
            this->stopSound();
        }

        void setFileName(const std::string filename) {
            this->soundFileName = filename;
        }

        void setAudioEngine(ma_engine *audioEngine) {
            this->audioEngine = audioEngine;
        }

        /** Plays a the sound set in this AudioNode */
        virtual void playSound() {
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

            // Play
            ma_sound_set_volume(&this->sound, this->volume);
            ma_sound_start(&this->sound);

            log(std::format("Playing [{}] from [{}]", this->soundFileName, this->UUID));
        }

        /** Stops a sound being played, if any is */
        void stopSound() {
            if (!this->isInitialized)
                return;

            ma_sound_uninit(&this->sound);
            this->isInitialized = false;
            log(std::format("Stopped [{}] from [{}]", this->soundFileName, this->UUID));
        }

        /** Enables sound looping */
        void enableLooping() {
            if (!this->isInitialized)
                return;

            ma_sound_set_looping(&this->sound, MA_TRUE);
        }

        /** Disables sound looping */
        void disableLooping() {
            if (!this->isInitialized)
                return;

            ma_sound_set_looping(&this->sound, MA_FALSE);
        }

        void setVolume(const float volume) {
            this->volume = volume;

            if (!this->isInitialized)
                return;

            ma_sound_set_volume(&this->sound, volume);
        }


};

#endif
