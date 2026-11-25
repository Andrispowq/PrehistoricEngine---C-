#pragma once

#include "platform/vulkan/buffer/VKBuffer.h"
#include <array>

namespace Prehistoric {
    class VKRenderer;
    class VKRenderpass;

    class VKVoxelView {
    public:
        VKVoxelView(VKDevice *gpu, VKRenderpass &passes, VkRenderPass output, uint32_t resolution, bool lighting);

        ~VKVoxelView();

        void Render(VkCommandBuffer cmd, VKRenderer &renderer, VkRenderPass pass, VkFramebuffer frame,
                    VkExtent2D extent);

        void Prepare(VkCommandBuffer cmd, VKRenderer &renderer);

        VkDescriptorSet GridSet() const { return viewInputs; }
        VkDescriptorSetLayout GridLayout() const { return viewSet; }

        enum Timing {
            Geometry,
            Injection,
            Bounces,
            Mips,
            Trace,
            Shading,
            TimingCount
        };

        void Mark(VkCommandBuffer cmd, Timing stage, bool end);

        static size_t GridBytes(unsigned resolution, bool lighting);

        VkBuffer Buffer() const { return cells->getBuffer(); }
        const uint32_t resolution;
        const bool lighting;

    private:
        using Vec4 = std::array<float, 4>;

        struct Surface {
            std::array<float, 16> model;
            Vec4 grid, colour, material;
            std::array<uint32_t, 4> range;
        };

        struct Draw {
            Surface surface;
            VkBuffer vertices, indices;
            VkImageView albedo;
            VkSampler sampler, metallicSampler;
            VkImageView metallic;

            bool operator==(const Draw &other) const;
        };

        std::vector<Draw> Collect(VKRenderer &renderer, const Vec4 &grid) const;

        void Bake(VkCommandBuffer cmd, const std::vector<Draw> &draws);

        void Release();

        VkDevice device;
        VkQueryPool timers{};
        double timestampMs = 0;
        uint64_t timestampMask = 0;
        bool recorded[TimingCount]{};

        void StartTimings(VkCommandBuffer cmd, VKRenderer &renderer);

        std::unique_ptr<VKBuffer> cells;
        VkDescriptorPool pool{};
        VkDescriptorSetLayout bakeSet{}, viewSet{};
        VkPipelineLayout bakeLayout{}, viewLayout{};
        VkPipeline bakePipeline{}, viewPipeline{}, lightPipeline{};
        VkPipelineLayout lightLayout{};
        VkDescriptorSetLayout lightSet{};
        VkDescriptorSet lightInputs{};
        Vec4 previousSun{}, previousDirection{};
        int previousBounces = -1;
        bool previousDirectional = false, previousVoxelShadows = false;
        std::vector<std::pair<VkBuffer, std::array<float, 16> > > previousCasters;
        VkDescriptorSet viewInputs{};
        std::vector<VkDescriptorSet> sets;
        std::vector<Draw> previous;
    };
}