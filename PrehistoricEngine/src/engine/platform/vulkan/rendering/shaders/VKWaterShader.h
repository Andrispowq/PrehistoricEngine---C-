#pragma once
#include "VKShader.h"

namespace Prehistoric
{
    class VKRenderer;
    class VKWaterShader : public VKShader
    {
    public:
        explicit VKWaterShader(Window* window);
        void BindGlobalSets() const override { BindSets(commandBuffer,0,1); }
        void BindTextureSets(uint32_t index) const override { BindSets(commandBuffer,1,1,index); }
        void BindObjectSets(uint32_t index) const override { BindSets(commandBuffer,2,1,index); }
        void UpdateGlobalUniforms(Renderer* renderer) const override;
        void UpdateMaterialUniforms(Material* material,uint32_t index) const override;
        void UpdateObjectUniforms(GameObject* object,uint32_t index) const override;
    private:
        mutable VKRenderer* renderer = nullptr;
    };
}
