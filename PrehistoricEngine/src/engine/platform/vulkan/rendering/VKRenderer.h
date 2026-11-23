#ifndef VK_RENDERER_H
#define VK_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include "platform/vulkan/rendering/command/VKCommandPool.h"
#include "platform/vulkan/rendering/renderpass/VKRenderpass.h"

namespace Prehistoric {
    class VKEnvironmentBackground;
    class VKEnvironmentLighting;

    class VKRenderer : public Renderer {
    public:
        VKRenderer(Window* window, Camera* camera, AssembledAssetManager* manager);

        virtual ~VKRenderer();

        virtual void PrepareRendering();

        virtual void EndRendering();

        virtual void Render() override;

        void BuildCommandBuffers();

        virtual CommandBuffer* getDrawCommandBuffer() const {
            return (CommandBuffer*) commandPool->getCommandBuffer(window->getSwapchain()->getAquiredImageIndex());
        }

        VKEnvironmentLighting* getEnvironmentLighting() const {
            return environmentLighting.get();
        }

    private:
        std::unique_ptr<VKCommandPool> commandPool;
        std::unique_ptr<VKRenderpass> renderpass;

        std::vector<std::unique_ptr<VKFramebuffer>> primaryFramebuffers;

        std::unique_ptr<VKEnvironmentBackground> background;
        std::unique_ptr<VKEnvironmentLighting> environmentLighting;

        bool commandBuffersReady = false;
        bool frameReady = false;
    };
};

#endif