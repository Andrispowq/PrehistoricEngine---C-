#ifndef FFT_H
#define FFT_H

#include "prehistoric/common/texture/Texture.h"
#include "prehistoric/common/rendering/shaders/Shader.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "H0k.h"
#include "Hkt.h"
#include "TwiddleFactors.h"

namespace Prehistoric {
    class FFT {
    public:
        FFT(AssembledAssetManager *manager, Window *window, WaterConfig *config);

        ~FFT();

        void Render();

        float GetTime() const { return t; }

        void SetTime(float value) {
            t = value;
            systemTime = Time::getTimeFromStartNanoseconds() / 1000000;
        }

        TextureHandle getDyTexture() const { return Dy; }
        TextureHandle getDxTexture() const { return Dx; }
        TextureHandle getDzTexture() const { return Dz; }

    private:
        AssembledAssetManager *manager = nullptr;
        Window *window = nullptr;

        TextureHandle Dx;
        TextureHandle Dy;
        TextureHandle Dz;
        bool choppy = false;

        ShaderHandle butterflyShader;
        ShaderHandle inversionShader;
        TwiddleFactors *twiddleFactors;

        PipelineHandle butterflyPipeline;
        PipelineHandle inversionPipeline;

        TextureHandle pingpongTexture;

        H0k *h0k = nullptr;
        Hkt *hkt = nullptr;

        float t_delta = 0.0f;
        float t = 0.0f;
        uint64_t systemTime = 0;
        int pingpong = 0;
        int log_2_N = 0;
        int N = 0;
    };
}

#endif