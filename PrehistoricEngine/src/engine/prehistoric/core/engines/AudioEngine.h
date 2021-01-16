#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "Engine.h"

#include <al.h>
#include <alc.h>

#include "prehistoric/core/node/component/audio/AudioComponent.h"

namespace Prehistoric
{
    struct AudioBufferData
    {
        ALuint ID;
        int channels;
        int sampleRate;
        int bitsPerSample;
        int length;
    };

    class AudioEngine : public Engine
    {
    public:
        AudioEngine();
        virtual ~AudioEngine();

        void Input();

        virtual void OnEvent(Event& event) override;
        virtual void Update(float delta) override;

        void addAudioComponent(AudioComponent* audioComponent);

        static size_t loadBuffer(const std::string& audioFile);

        static size_t getIndexOf(const std::string& audioFile);
        static ALuint getBuffer(size_t bufferIndex) { return buffers[bufferIndex].ID; }

        static char* loadWAV(const char* fn, int& chan, int& samplerate, int& bps, int& size);
        static int getFormat(uint16_t channels, uint16_t bitsPerSample);

    private:
        ALCdevice* device;
        ALCcontext* context;

        std::vector<AudioComponent*> audioComponents;

        static std::vector<AudioBufferData> buffers;
        static std::map<std::string, size_t> audioIndices;
    };
};

#endif