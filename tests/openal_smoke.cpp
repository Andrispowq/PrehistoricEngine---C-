#include <AL/al.h>
#include <AL/alc.h>
#include <cstdio>

// Run with ALSOFT_DRIVERS=null for a silent, hardware-independent link/API check.
int main()
{
    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) return 1;
    ALCcontext* context = alcCreateContext(device, nullptr);
    if (!context || !alcMakeContextCurrent(context))
    {
        if (context) alcDestroyContext(context);
        alcCloseDevice(device);
        return 2;
    }

    ALuint buffer = 0;
    alGenBuffers(1, &buffer);
    const short silence[64] = {};
    alBufferData(buffer, AL_FORMAT_MONO16, silence, sizeof(silence), 44100);
    const bool success = alGetError() == AL_NO_ERROR;
    if (buffer) alDeleteBuffers(1, &buffer);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
    if (!success) return 3;
    std::puts("OpenAL context and audio buffer OK");
    return 0;
}
