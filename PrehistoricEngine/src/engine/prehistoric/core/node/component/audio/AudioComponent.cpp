#include "engine/prehistoric/core/util/Includes.hpp"
#include "AudioComponent.h"

#include "engine/prehistoric/core/Engine.h"

AudioComponent::AudioComponent(size_t audioBuffer, bool is3D, bool isStereo, bool isLoop)
{
	alGenSources(1, &sourceID);

    setGain(1.0f);
    setPitch(1.0f);
    setLoop(isLoop);
    set3D(is_3D);
    setStereo(is_Stereo);
    setLoop(is_Loop);
    setBufferIndex(audioBuffer);
}

AudioComponent::AudioComponent(const std::string& audio, bool is3D, bool isStereo, bool isLoop)
{
    alGenSources(1, &sourceID);

    setGain(1.0f);
    setPitch(1.0f);
    setLoop(isLoop);
    set3D(is_3D);
    setStereo(is_Stereo);
    setLoop(is_Loop);
    setBufferIndex(AudioEngine::loadBuffer(audio));
}

AudioComponent::~AudioComponent()
{
    alDeleteSources(1, &sourceID);
}

void AudioComponent::PreUpdate(Engine* engine)
{
    Vector3f pos = parent->getWorldTransform().getPosition();
    alSource3f(sourceID, AL_POSITION, pos.x, pos.y, pos.z);

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
}
