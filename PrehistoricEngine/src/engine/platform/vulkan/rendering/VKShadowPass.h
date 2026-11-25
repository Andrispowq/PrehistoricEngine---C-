#pragma once

#include "platform/vulkan/texture/VKImage.h"
#include "platform/vulkan/rendering/framebuffer/VKFramebuffer.h"
#include "prehistoric/common/rendering/ShadowCascades.h"

namespace Prehistoric {
    class VKCommandBuffer;

    class VKShadowPass {
    public:
        explicit VKShadowPass(VKDevice *gpu) : gpu(gpu) {
        }

        ~VKShadowPass();

        void Prepare(Renderer &renderer, float aspect);

        void Record(Renderer &renderer, VKCommandBuffer *buffer);

        Matrix4f RecordVoxelShadow(Renderer &renderer, VkCommandBuffer cmd, Vector3f center, float extent, bool draw);

        VkDescriptorImageInfo VoxelDepth() const;

        VkDescriptorImageInfo GetDepth() const;

        struct Uniforms {
            float matrices[ShadowCascades::Count][16];
            float view[16];
            float splits[8];
            float direction[4];
            float radiance[4];
            float settings[4];
        };

        const Uniforms &GetUniforms() const { return uniforms; }
        const Light *GetSun() const { return sun; }

    private:
        void DrawLayer(Renderer &renderer, VkCommandBuffer cmd, VKFramebuffer &frame, unsigned width,
                       const Matrix4f &light, bool draw);

        std::unique_ptr<VKImage> voxelDepth;
        std::unique_ptr<VKFramebuffer> voxelFrame;
        bool voxelDepthReady = false;

        void CreateTargets(unsigned size);

        void CreatePipeline();

        void ReleaseTargets();

        VKDevice *gpu;
        ShadowCascades cascades;
        Uniforms uniforms{};
        const Light *sun{};
        unsigned resolution{};
        bool initialized{};
        std::unique_ptr<VKImage> image;
        VkImageView views[ShadowCascades::Count]{};
        std::unique_ptr<VKFramebuffer> frames[ShadowCascades::Count];
        VkSampler sampler{};
        VkRenderPass pass{};
        VkPipelineLayout layout{};
        VkPipeline pipeline{};
    };
}