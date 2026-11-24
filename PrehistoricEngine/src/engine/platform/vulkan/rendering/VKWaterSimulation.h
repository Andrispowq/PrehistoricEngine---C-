#pragma once
#include "platform/vulkan/texture/VKTexture.h"
#include "prehistoric/core/resources/AssetManager.h"

namespace Prehistoric
{
    class VKWaterSimulation
    {
    public:
        VKWaterSimulation(VKDevice* gpu, AssetManager* assets);
        ~VKWaterSimulation();
        void Record(VkCommandBuffer command, float seconds);
        VKTexture* displacement(unsigned axis) const { return outputs[axis].get(); }
        VKTexture* normals() const { return normal.get(); }
    private:
        struct Kernel;
        std::unique_ptr<Kernel> MakeKernel(const char* name, std::initializer_list<VKTexture*> images, unsigned storageCount);
        VKDevice* gpu;
        AssetManager* assets;
        int size, stages = 0;
        bool initialized = false;
        std::array<TextureHandle, 4> noise{};
        std::unique_ptr<VKTexture> initial[2], coefficients[3], outputs[3], scratch, twiddles, normal;
        std::unique_ptr<Kernel> spectrum, evolve, factors, butterfly[3], inversion[3], normalPass;
    };
}

