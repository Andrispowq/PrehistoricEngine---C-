#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <al.h>
#include <alc.h>

#include "engine/prehistoric/core/node/component/audio/AudioComponent.h"

typedef struct WAV_HEADER
{
    // RIFF Chunk Descriptor
    uint8_t         RIFF[4];        // RIFF Header Magic header
    uint32_t        ChunkSize;      // RIFF Chunk Size
    uint8_t         WAVE[4];        // WAVE Header
    // "fmt" sub-chunk
    uint8_t         fmt[4];         // FMT header
    uint32_t        Subchunk1Size;  // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
    uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
    uint32_t        bytesPerSec;    // bytes per second
    uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    uint16_t        bitsPerSample;  // Number of bits per sample
    // "data" sub-chunk
    uint8_t         Subchunk2ID[4]; // "data"  string
    uint32_t        Subchunk2Size;  // Sampled data length
} wav_header;

class AudioEngine
{
public:
	AudioEngine();
	virtual ~AudioEngine();

	void Input();
	void Update(float delta);

    void addAudioComponent(AudioComponent* audioComponent);

    static size_t loadBuffer(const std::string& audioFile);

    static size_t getIndexOf(const std::string& audioFile);
    static ALuint getBuffer(size_t bufferIndex) { return buffers[bufferIndex]; }

    static char* load_wav(const std::string& filename, wav_header& header);
    static int getFormat(uint16_t channels, uint16_t bitsPerSample);
    
private:
	ALCdevice* device;
	ALCcontext* context;

    std::vector<AudioComponent*> audioComponents;

    static std::vector<ALuint> buffers;
    static std::map<std::string, size_t> audioIndices;
};

#endif