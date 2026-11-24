#ifndef VK_RENDERPASS_H
#define VK_RENDERPASS_H

#include <vulkan/vulkan.h>
#include "platform/vulkan/texture/VKTexture.h"

#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/rendering/framebuffer/VKFramebuffer.h"
#include "platform/vulkan/rendering/command/VKCommandBuffer.h"

namespace Prehistoric {
    class Water;
    class VKSwapchain;
    class VKImage;

    class VKRenderpass {
    public:
        VKRenderpass(VKDevice *device, VKSwapchain *swapchain);

        virtual ~VKRenderpass();

        void BeginRenderpass(VKCommandBuffer *commandBuffer, Vector4f clearColour, VkFramebuffer target = VK_NULL_HANDLE);

        struct WaterTargets
        {
            std::unique_ptr<VKTexture> reflection, refraction;
            std::unique_ptr<VKFramebuffer> reflectionFrame, refractionFrame;
        };
        WaterTargets& GetWaterTargets(const Water* water);
        void RetainWaterTargets(const std::vector<Water*>& active);

        Texture *getEditorOutput() const { return editorOutput.get(); }
        VkRenderPass getPresentPass() const { return presentPass; }

        void RenderHDR(VKCommandBuffer *commandBuffer, bool bloomEnabled, bool underwater = false);

        VkPipeline CreateFullscreenPipeline(const char *vertexPath, const char *fragmentPath, VkPipelineLayout layout,
                                            VkRenderPass postPass = VK_NULL_HANDLE);

        VkRenderPass getRenderPass() const { return renderpass; }

    private:
        struct BloomDraw {
            VkFramebuffer framebuffer;
            VkExtent2D extent;
            VkDescriptorSet inputs;
            float settings[4];
        };

        void CreateScenePass();

        void CreatePostProcessing();

        VkRenderPass CreateOutputPass(VkFormat format, VkImageLayout finalLayout);

        VkDescriptorSet CreateInputs(VkImageView source, VkImageView second);

        VKImage *CreateImage(VkExtent2D extent, VkSampleCountFlagBits samples);

        VKFramebuffer *CreateFramebuffer(VkRenderPass pass, VkExtent2D extent, uint32_t count, VkImageView *views);

        std::unique_ptr<VKTexture> editorOutput;
        VkFramebuffer editorFramebuffer{};
        VkRenderPass editorPass{};
        VkRenderPass renderpass{};
        VkRenderPass bloomPass{};
        VkRenderPass presentPass{};
        VkDescriptorSetLayout hdrSetLayout{};
        VkDescriptorPool hdrPool{};
        VkSampler sampler{};
        VkPipelineLayout hdrLayout{};
        VkPipeline hdrPipeline{};
        VkPipeline bloomPipeline{};
        VkDescriptorSet hdrInputs[2]{};
        std::unordered_map<const Water*, WaterTargets> waterTargets;
        VKImage* emissionTarget{};
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