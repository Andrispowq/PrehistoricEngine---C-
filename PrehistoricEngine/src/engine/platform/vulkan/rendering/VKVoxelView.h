#pragma once

#include "platform/vulkan/buffer/VKBuffer.h"
#include <array>

namespace Prehistoric {
    class VKRenderer;
    class VKRenderpass;

    class VKVoxelView {
    public:
        VKVoxelView(VKDevice *gpu, VKRenderpass &passes, VkRenderPass output);

        ~VKVoxelView();

        void Render(VkCommandBuffer cmd, VKRenderer &renderer, VkRenderPass pass, VkFramebuffer frame,
                    VkExtent2D extent);

    private:
        using Vec4 = std::array<float, 4>;

        struct Surface {
            std::array<float, 16> model;
            Vec4 grid, colour;
            std::array<uint32_t, 4> range;
        };

        struct Draw {
            Surface surface;
            VkBuffer vertices, indices;
            VkImageView albedo;
            VkSampler sampler;

            bool operator==(const Draw &other) const;
        };

        std::vector<Draw> Collect(VKRenderer &renderer, const Vec4 &grid) const;

        void Bake(VkCommandBuffer cmd, const std::vector<Draw> &draws);

        void Release();

        VkDevice device;
        std::unique_ptr<VKBuffer> cells;
        VkDescriptorPool pool{};
        VkDescriptorSetLayout bakeSet{}, viewSet{};
        VkPipelineLayout bakeLayout{}, viewLayout{};
        VkPipeline bakePipeline{}, viewPipeline{};
        VkDescriptorSet viewInputs{};
        std::vector<VkDescriptorSet> sets;
        std::vector<Draw> previous;
    };
}