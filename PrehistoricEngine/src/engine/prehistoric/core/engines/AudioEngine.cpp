#include "Includes.hpp"
#include "AudioEngine.h"

namespace Prehistoric
{
    std::vector<ALuint> AudioEngine::buffers;
    std::map<std::string, size_t> AudioEngine::audioIndices;

    AudioEngine::AudioEngine()
    {
        device = alcOpenDevice(nullptr);
        if (!device)
        {
            PR_LOG_ERROR("Cannot create OpenAL device!\n");
        }

        context = alcCreateContext(device, nullptr);
        if (!context)
        {
            PR_LOG_ERROR("Cannot create OpenAL context!\n");
        }

        if (!alcMakeContextCurrent(context))
        {
            PR_LOG_ERROR("Cannot make the OpenAL context current!\n");
        }
    }

    AudioEngine::~AudioEngine()
    {
        alDeleteBuffers((ALsizei)buffers.size(), buffers.data());

        alcDestroyContext(context);

        if (!alcCloseDevice(device))
        {
            PR_LOG_ERROR("Cannot close the OpenAL device!\n");
        }
    }

    void AudioEngine::Input()
    {
    }

    void AudioEngine::OnEvent(Event& event)
    {
    }

    void AudioEngine::Update(float delta)
    {
       for (auto comp : audioComponents)
       {
           uint32_t id = comp->getSourceID();

           int state;
           alGetSourcei(id, AL_SOURCE_STATE, &state);

           if (state == AL_PLAYING)
               continue;

           alSourcePlay(id);
       }

       audioComponents.clear();
    }

    void AudioEngine::addAudioComponent(AudioComponent* audioComponent)
    {
        audioComponents.push_back(audioComponent);
    }

    size_t AudioEngine::loadBuffer(const std::string& audioFile)
    {
        wav_header header;
        char* soundData = load_wav(audioFile, header);

        if (!soundData)
        {
            PR_LOG_ERROR("ERROR: Could not load wav\n");
        }

        ALuint buffer;
        alGenBuffers(1, &buffer);
        alBufferData(buffer, getFormat(header.NumOfChan, header.bitsPerSample), soundData, header.Subchunk2Size, header.SamplesPerSec);
        delete[] soundData;

        buffers.push_back(buffer);

        size_t index = buffers.size() - 1;
        audioIndices.emplace(std::make_pair(audioFile, index));

        return index;
    }

    size_t AudioEngine::getIndexOf(const std::string& audioFile)
    {
        auto index = audioIndices.find(audioFile);
        if (index == audioIndices.end())
        {
            return -1;
        }

        return index->second;
    }

    char* AudioEngine::load_wav(const std::string& filename, wav_header& header)
    {
        size_t headerSize = sizeof(wav_header), filelength = 0;

        std::ifstream file(filename.c_str(), std::ios::ate | std::ios::binary);
        size_t fileSize = file.tellg();
        file.seekg(0);

        if (!file.is_open())
        {
            PR_LOG_ERROR("Unable to open wave file: %s\n", filename.c_str());
            return nullptr;
        }

        //Read the header
        char* header_data = new char[headerSize];
        file.read(header_data, headerSize);
        memcpy(&header, (void*)header_data, headerSize); //Copying the loaded bytes - not safe, but it will be okay for now

        char* data = new char[header.Subchunk2Size];
        file.read(data, header.Subchunk2Size);

        return data;
    }

    int AudioEngine::getFormat(uint16_t channels, uint16_t bitsPerSample)
    {
        if (channels == 1 && bitsPerSample == 8)
            return AL_FORMAT_MONO8;
        else if (channels && bitsPerSample == 16)
            return AL_FORMAT_MONO16;
        else if (channels && bitsPerSample == 8)
            return AL_FORMAT_STEREO8;
        else if (channels && bitsPerSample == 16)
            return AL_FORMAT_STEREO16;
        else
            return -1;
    }
};
