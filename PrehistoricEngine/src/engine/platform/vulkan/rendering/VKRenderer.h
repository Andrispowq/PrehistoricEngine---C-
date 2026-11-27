#ifndef VK_RENDERER_H
#define VK_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"
#include "prehistoric/core/modules/atmosphere/AtmosphereCapture.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include "platform/vulkan/rendering/command/VKCommandPool.h"
#include "platform/vulkan/rendering/renderpass/VKRenderpass.h"

namespace Prehistoric {
    class VKShader;
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

        RenderExtent GetRenderExtent() const override;

        std::vector<unsigned> GetSupportedSampleCounts() const override;

        unsigned GetSampleCount() const override;


        VKCommandBuffer *getDrawCommandBuffer() const {
            return commandPool->getCommandBuffer(window->getSwapchain()->getAquiredImageIndex());
        }

        VKShadowPass &GetShadows() { return *shadows; }
        const VKShadowPass &GetShadows() const { return *shadows; }

        bool UseTemporalEffects() const override { return Renderer::UseTemporalEffects() && !voxelView; }

        bool voxelView = false;
        bool voxelHalfResolution = true;
        bool voxelGI = false, voxelDirectional = true, voxelSunShadows = false;
        int voxelResolution = 128, voxelBounces = 4;
        double voxelMilliseconds[6]{-1, -1, -1, -1, -1, -1};
        float voxelStrength = 1.0f;

        VkBuffer GetVoxelBuffer() const;

        VkBuffer GetVoxelGeometryBuffer(unsigned cascade = 0) const;

        VkDescriptorImageInfo GetVoxelScreenInput(int index) const;

        bool SupportsVoxelResolution(int size) const;

        Vector3f voxelCenter{0.0f};
        float voxelExtent = 256.0f;
        Vector4f GetVoxelVolume() const override { return Vector4f(voxelCenter, voxelExtent); }

        void SetVoxelVolume(const Vector3f &center, float width, bool resetHistory = true) override {
            voxelCenter = center;
            voxelExtent = std::max(1.0f, width);
            if (resetHistory) ResetTemporalHistory();
        }

        void RequestEnvironmentBake() override;

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
            uint32_t selected = 0;
        };

        static_assert(offsetof(CaptureConstants, eyeAndCapture) == 64);
        static_assert(offsetof(CaptureConstants, selected) == 80);

        struct FrameDescriptors {
            std::unordered_map<Material *, uint32_t> materials;
            uint32_t objects = 0;
        };

        std::unordered_map<VKShader *, FrameDescriptors> frameDescriptors;

        void PrepareDescriptors();

        void ProcessDraws(VKCommandBuffer *buffer, const CaptureConstants *capture = nullptr, bool receivers = false);

        std::vector<Water *> waters;

        bool CanRenderFrame() const override { return frameReady; }

        void UpdateEnvironmentLighting() override;

        void RenderScene() override;

        void RenderHDR() override;

        void FinishFrame() override;

        AtmosphereCapture environmentCapture;
        bool bakeRequested = false;
        bool frameReady = false;
    };
};

#endif