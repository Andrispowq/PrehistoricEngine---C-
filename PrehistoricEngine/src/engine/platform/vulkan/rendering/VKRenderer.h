#ifndef VK_RENDERER_H
#define VK_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"
#include "prehistoric/core/modules/atmosphere/AtmosphereCapture.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include "platform/vulkan/rendering/command/VKCommandPool.h"
#include "platform/vulkan/rendering/renderpass/VKRenderpass.h"

namespace Prehistoric {
    class VKShadowPass;
    class Water;
    class VKEnvironmentBackground;
    class VKEnvironmentLighting;

    class VKRenderer : public Renderer {
    public:
        VKRenderer(Window *window, Camera *camera, AssembledAssetManager *manager);

        virtual ~VKRenderer();

        virtual void PrepareRendering();

        virtual void EndRendering();


        VKCommandBuffer *getDrawCommandBuffer() const {
            return commandPool->getCommandBuffer(window->getSwapchain()->getAquiredImageIndex());
        }

        const VKShadowPass &GetShadows() const { return *shadows; }

        void RequestEnvironmentBake(bool usePanorama) override;

        Texture *getOutputTexture() const override;

        void RegisterWater(Water *water) { waters.push_back(water); }
        VKRenderpass::WaterTargets &GetWaterTargets(const Water *water) { return renderpass->GetWaterTargets(water); }

        VKEnvironmentLighting *getEnvironmentLighting() const {
            return environmentLighting.get();
        }

    private:
        std::unique_ptr<VKShadowPass> shadows;
        std::unique_ptr<VKCommandPool> commandPool;
        std::unique_ptr<VKRenderpass> renderpass;

        std::unique_ptr<VKEnvironmentBackground> background;
        std::unique_ptr<VKEnvironmentLighting> environmentLighting;

        struct CaptureConstants {
            float worldToClip[16];
            float eyeAndCapture[4];
        };

        static_assert(offsetof(CaptureConstants, eyeAndCapture) == 64);
        static_assert(sizeof(CaptureConstants) == 80);

        void ProcessDraws(VKCommandBuffer *buffer, const CaptureConstants *capture = nullptr);

        std::vector<Water *> waters;

        bool CanRenderFrame() const override { return frameReady; }

        void UpdateEnvironmentLighting() override;

        void RenderScene() override;

        void RenderHDR() override;

        void FinishFrame() override;

        AtmosphereCapture environmentCapture;
        bool panoramaOverride = false;
        bool bakeRequested = false;
        bool frameReady = false;
    };
};

#endif