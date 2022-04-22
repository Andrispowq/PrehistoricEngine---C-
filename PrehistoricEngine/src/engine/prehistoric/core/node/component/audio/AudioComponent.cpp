#include "Includes.hpp"
#include "AudioComponent.h"

#include "prehistoric/core/CoreEngine.h"

#include <AL/alext.h>

namespace Prehistoric
{
    AudioComponent::AudioComponent(size_t audioBuffer, float startOffset, bool is3D, bool isStereo, bool isLoop)
    {
        type = ComponentType::AudioComponent;
        this->startOffset = startOffset;

        alGenSources(1, &sourceID);

        setGain(1.0f);
        setPitch(1.0f);
        setLoop(isLoop);
        set3D(is3D);
        setStereo(isStereo);
        setLoop(isLoop);
        setSpatial(true);
        setBufferIndex(audioBuffer);
    }

    AudioComponent::AudioComponent(const std::string& audio, float startOffset, bool is3D, bool isStereo, bool isLoop)
    {
        type = ComponentType::AudioComponent;
        this->startOffset = startOffset;

        alGenSources(1, &sourceID);

        setGain(1.0f);
        setPitch(1.0f);
        setLoop(isLoop);
        set3D(is3D);
        setStereo(isStereo);
        setLoop(isLoop);
        setSpatial(true);
        setBufferIndex(AudioEngine::loadBuffer(audio));
    }

    AudioComponent::~AudioComponent()
    {
        alDeleteSources(1, &sourceID);
    }

    void AudioComponent::PreUpdate(CoreEngine* engine)
    {
        engine->getAudioEngine()->addAudioComponent(this);
    }

    void AudioComponent::setGain(float gain)
    {
        alSourcef(sourceID, AL_GAIN, gain);
    }

    void AudioComponent::setPitch(float pitch)
    {
        alSourcef(sourceID, AL_PITCH, pitch);
    }

    void AudioComponent::setVelocity(const Vector3f& velocity)
    {
        alSource3f(sourceID, AL_VELOCITY, 0, 0, 0);
    }

    void AudioComponent::setBufferIndex(size_t bufferIndex)
    {
        this->bufferIndex = bufferIndex;

        ALuint buffer = AudioEngine::getBuffer(bufferIndex);
        alSourcei(sourceID, AL_BUFFER, buffer);
    }

    void AudioComponent::set3D(bool is_3D)
    {
        this->is_3D = is_3D;
    }

    void AudioComponent::setStereo(bool is_Stereo)
    {
        this->is_Stereo = is_Stereo;
    }

    void AudioComponent::setLoop(bool is_Loop)
    {
        this->is_Loop = is_Loop;
        alSourcei(sourceID, AL_LOOPING, is_Loop ? AL_TRUE : AL_FALSE);
    }
};
