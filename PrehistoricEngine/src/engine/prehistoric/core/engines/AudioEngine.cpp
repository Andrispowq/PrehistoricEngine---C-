#include "Includes.hpp"
#include "AudioEngine.h"

#include <AL/alext.h>

//#include "AudioFile.h"

namespace Prehistoric
{
    std::vector<AudioBufferData> AudioEngine::buffers;
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

        float pos[] = { 0, 0, 0 };
        float vel[] = { 0, 0, 0 };
        float ori[] = { 0.0,0.0,-1.0, 0.0,1.0,0.0 };

        alListenerfv(AL_POSITION, pos);
        alListenerfv(AL_VELOCITY, vel);
        alListenerfv(AL_ORIENTATION, ori);

        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    }

    AudioEngine::~AudioEngine()
    {
        std::vector<ALuint> bufs;
        for (auto elem : buffers)
        {
            bufs.push_back(elem.ID);
        }

        alDeleteBuffers((ALsizei)buffers.size(), bufs.data());

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

           alSourcef(id, AL_SEC_OFFSET, comp->getStartOffset());
           alSourcei(id, AL_SOURCE_SPATIALIZE_SOFT, comp->isSpatial() ? AL_TRUE : AL_FALSE);

           if (comp->is3D())
           {
               Vector3f pos = comp->getParent()->getWorldTransform().getPosition();
               alSource3f(id, AL_POSITION, pos.x, pos.y, pos.z);               
           }

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
        int channels, sampleRate, bitsPerSample, size;
        char* data = loadWAV(audioFile.c_str(), channels, sampleRate, bitsPerSample, size);

        if (!data)
        {
            PR_LOG_ERROR("ERROR: Could not load wav\n");
        }

        ALuint buffer;
        alGenBuffers(1, &buffer);
        alBufferData(buffer, getFormat(channels, bitsPerSample), data, size, sampleRate);

        AudioBufferData buffData;
        buffData.ID = buffer;
        buffData.channels = channels;
        buffData.sampleRate = sampleRate;
        buffData.bitsPerSample = bitsPerSample;
        buffData.length = int(size / sampleRate);

        buffers.push_back(buffData);

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

    bool isBigEndian()
    {
        int a = 1;
        return !((char*)&a)[0];
    }

    int convertToInt(char* buffer, int len)
    {
        int a = 0;
        if (!isBigEndian())
            for (int i = 0; i < len; i++)
                ((char*)&a)[i] = buffer[i];
        else
            for (int i = 0; i < len; i++)
                ((char*)&a)[3 - i] = buffer[i];
        return a;
    }

    char* AudioEngine::loadWAV(const char* fn, int& chan, int& samplerate, int& bps, int& size)
    {
        enum class WavChunks 
        {
            RiffHeader = 0x46464952,
            WavRiff = 0x54651475,
            Format = 0x020746d66,
            LabeledText = 0x478747C6,
            Instrumentation = 0x478747C6,
            Sample = 0x6C706D73,
            Fact = 0x47361666,
            Data = 0x61746164,
            Junk = 0x4b4e554a,
        };

        enum class WavFormat 
        {
            PulseCodeModulation = 0x01,
            IEEEFloatingPoint = 0x03,
            ALaw = 0x06,
            MuLaw = 0x07,
            IMAADPCM = 0x11,
            YamahaITUG723ADPCM = 0x16,
            GSM610 = 0x31,
            ITUG721ADPCM = 0x40,
            MPEG = 0x50,
            Extensible = 0xFFFE
        };

        char buffer[4];
        std::ifstream in(fn, std::ios::binary);

        WavFormat format;
        char* data;

        int formatSize, formatblockalign, bitspersecond, extradata, headerid, memsize, riffstyle, skipsize;

        int chunkid = 0;
        bool datachunk = false;
        while (!datachunk)
        {
            in.read(buffer, 4);
            chunkid = convertToInt(buffer, 4);
            switch ((WavChunks)chunkid) 
            {
            case WavChunks::Format:
                in.read(buffer, 4);
                formatSize = convertToInt(buffer, 4);
                in.read(buffer, 2);
                format = (WavFormat)convertToInt(buffer, 2);
                in.read(buffer, 2);
                chan = convertToInt(buffer, 2);
                in.read(buffer, 4);
                samplerate = convertToInt(buffer, 4);
                in.read(buffer, 4);
                bitspersecond = convertToInt(buffer, 4);
                in.read(buffer, 2);
                formatblockalign = convertToInt(buffer, 2);
                in.read(buffer, 2);
                bps = convertToInt(buffer, 2);
                if (formatSize == 18)
                {
                    in.read(buffer, 2);
                    extradata = convertToInt(buffer, 2);
                    in.seekg(extradata + in.tellg());
                }
                break;
            case WavChunks::RiffHeader:
                headerid = chunkid;
                in.read(buffer, 4);
                memsize = convertToInt(buffer, 4);
                in.read(buffer, 4);
                riffstyle = convertToInt(buffer, 4);
                break;
            case WavChunks::Data:
                datachunk = true;
                in.read(buffer, 4);
                size = convertToInt(buffer, 4);
                break;
            default:
                in.read(buffer, 4);
                skipsize = convertToInt(buffer, 4);
                in.seekg(skipsize + in.tellg());
                break;
            }
        }

        data = new char[size];
        in.read(data, size);

        return data;
    }

    /*char* AudioEngine::loadWAV(const char* fn, int& chan, int& samplerate, int& bps, int& size)
    {
        char buffer[4];
        std::ifstream in(fn, std::ios::binary);
        in.read(buffer, 4);

        if (strncmp(buffer, "RIFF", 4) != 0)
        {
            std::cout << "this is not a valid WAVE file" << std::endl;
            return NULL;
        }

        in.read(buffer, 4);
        in.read(buffer, 4);      //WAVE
        in.read(buffer, 4);      //fmt
        in.read(buffer, 4);      //16
        in.read(buffer, 2);      //1
        in.read(buffer, 2);
        chan = convertToInt(buffer, 2);

        in.read(buffer, 4);
        samplerate = convertToInt(buffer, 4);

        in.read(buffer, 4);
        in.read(buffer, 2);
        in.read(buffer, 2);
        bps = convertToInt(buffer, 2);

        in.read(buffer, 4);     //cue
        if (buffer[0] = 'c' && buffer[1] == 'u' && buffer[2] == 'e' && buffer[3] == ' ')
        {
            in.read(buffer, 4);
            uint32_t ignoresize = convertToInt(buffer, 4);
            in.ignore(ignoresize);

            in.read(buffer, 4);     //data
        }

        while (buffer[0] != 'd' && buffer[1] != 'a' && buffer[2] != 't' && buffer[3] != 'a')
        {
            in.read(buffer, 4);
        }

        if (buffer[0] != 'd' && buffer[1] != 'a' && buffer[2] != 't' && buffer[3] != 'a')
            return false;

        in.read(buffer, 4);
        size = convertToInt(buffer, 4);

        char* data = new char[size];
        in.read(data, size);

        return data;
    }*/

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
