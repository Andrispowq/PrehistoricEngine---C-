#ifndef VK_RENDERPASS_H
#define VK_RENDERPASS_H

#include <vulkan/vulkan.h>
#include "platform/vulkan/texture/VKTexture.h"

#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/rendering/framebuffer/VKFramebuffer.h"
#include "platform/vulkan/rendering/command/VKCommandBuffer.h"

namespace Prehistoric {
    class Water;
    class VKTemporalPass;
    class VKBuffer;
    class VKRenderer;
    class VKVoxelView;
    class VKSwapchain;
    class VKImage;

    class VKRenderpass {
    public:
        VKRenderpass(VKDevice *device, VKSwapchain *swapchain, VkImageView flareAtlas);

        virtual ~VKRenderpass();

        void BeginRenderpass(VKCommandBuffer *commandBuffer, Vector4f clearColour,
                             const VKFramebuffer *target = nullptr, bool sampleDepth = false);

        struct WaterTargets {
            std::unique_ptr<VKTexture> reflection, refraction;
            std::unique_ptr<VKFramebuffer> reflectionFrame, refractionFrame;
        };

        WaterTargets &GetWaterTargets(const Water *water);

        void RetainWaterTargets(const std::vector<Water *> &active);

        Texture *getEditorOutput() const { return editorOutput.get(); }

        void PrepareVoxelScreen(bool traceIndirect, bool withRadiance);

        void BeginVoxelReceivers(VKCommandBuffer *buffer, bool withRadiance);

        void BoundReflectionDepth(VKCommandBuffer *buffer, VKRenderer &renderer);

        void TraceVoxelScreen(VKCommandBuffer *buffer, VKRenderer &renderer);

        VkDescriptorImageInfo VoxelScreenInput(int index, bool half) const;

        VKVoxelView &Voxels(uint32_t resolution, bool lighting);

        VkRenderPass getPresentPass() const { return presentPass; }

        void RenderHDR(VKCommandBuffer *commandBuffer, VKRenderer &renderer, bool bloomEnabled, bool underwater);

        VkPipeline CreateFullscreenPipeline(const char *vertexPath, const char *fragmentPath, VkPipelineLayout layout,
                                            VkRenderPass postPass = VK_NULL_HANDLE);

        VkRenderPass getReceiverPass() const { return receiverPass; }

        VkRenderPass getRenderPass() const { return renderpass; }

    private:
        friend class VKTemporalPass;
        std::unique_ptr<VKTemporalPass> temporalPass;
        std::vector<VkDescriptorSet> temporalInputs;
        VkImageView bloomResult{};

        struct BloomDraw {
            VkFramebuffer framebuffer;
            VkExtent2D extent;
            VkDescriptorSet inputs;
            float settings[15];
        };

        std::unique_ptr<VKVoxelView> voxelPass;

        void RenderLitHDR(VKCommandBuffer *commandBuffer, VKRenderer &renderer, bool bloomEnabled, bool underwater);

        void CreateScenePass();

        void CreateVolumePass();

        void UpdateVolumeInputs(VKRenderer &renderer);

        VkDescriptorSetLayout volumeSetLayout{};
        VkPipelineLayout volumeLayout{};
        VkPipeline volumePipeline{};
        VkDescriptorPool volumePool{};
        VkSampler depthSampler{};
        VkFramebuffer volumeFramebuffer{};
        VkDescriptorSet volumeOutput{};
        std::vector<VkDescriptorSet> volumeSets;

        struct VolumeFrame {
            std::unique_ptr<VKBuffer> media;
            std::unique_ptr<VKBuffer> shadows;
        };

        std::vector<VolumeFrame> volumeBuffers;

        void CreatePostProcessing();

        VkRenderPass CreateOutputPass(VkFormat format, VkImageLayout finalLayout);

        VkDescriptorSet CreateInputs(VkImageView source, VkImageView second);

        VKImage *CreateImage(VkExtent2D extent, VkSampleCountFlagBits samples);

        VKFramebuffer *CreateFramebuffer(VkRenderPass pass, VkExtent2D extent, uint32_t count, VkImageView *views);

        std::unique_ptr<VKTexture> editorOutput;
        VkFramebuffer editorFramebuffer{};
        VkRenderPass editorPass{};
        VkRenderPass receiverPass{};
        VkImageView receiverRadiance{}, reflectionDepth{};
        VkRenderPass reflectionDepthPass{};
        VkFramebuffer reflectionDepthFrame{};
        VkPipelineLayout reflectionDepthLayout{};
        VkPipeline reflectionDepthPipeline{};
        VkRenderPass renderpass{};
        VkRenderPass sceneWithReadableDepth{};
        VkRenderPass bloomPass{};
        VkRenderPass presentPass{};
        VkDescriptorSetLayout hdrSetLayout{};
        VkDescriptorPool hdrPool{};
        VkImageView flareAtlas;
        VkSampler sampler{};
        VkPipelineLayout hdrLayout{};
        VkPipeline hdrPipeline{}, dofPipeline{};
        VkPipeline bloomPipeline{};
        VkDescriptorSet hdrInputs[2]{};
        VKFramebuffer *litReceiverFramebuffer{};
        VKFramebuffer *receiverFramebuffer{};
        VkImageView receiverImages[2]{}, indirectImages[2]{};
        VkFramebuffer indirectFrames[2]{};
        VkDescriptorSet receiverInputs{};
        VkPipelineLayout traceLayout{};
        VkPipeline tracePipeline{};
        std::unordered_map<const Water *, WaterTargets> waterTargets;
        // Sequential water captures share attachments whose contents are discarded.
        std::unique_ptr<VKImage> captureDepth;
        VKImage *captureEmission{};
        VKImage *captureMsaaColour{};
        VKImage *captureMsaaEmission{};
        VKImage *emissionTarget{};
        VKImage *emission{};
        VKFramebuffer *sceneFramebuffer{};
        std::vector<std::unique_ptr<VKImage> > images;
        std::vector<std::unique_ptr<VKFramebuffer> > framebuffers;
        std::vector<VkFramebuffer> presentFramebuffers;
        std::vector<BloomDraw> bloomDraws;

        VKDevice *device;
        VKSwapchain *swapchain;
    };
};

#endif