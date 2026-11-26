#ifndef VK_GRAPHICS_PIPELINE_H
#define VK_GRAPHICS_PIPELINE_H

#include "prehistoric/common/rendering/pipeline/GraphicsPipeline.h"
#include "VKPipeline.h"

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/rendering/renderpass/VKRenderpass.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"

#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric {
    class VKGraphicsPipeline : public VKPipeline, public GraphicsPipeline {
    public:
        VKGraphicsPipeline(Window *window, AssetManager *manager, ShaderHandle shader, VertexBufferHandle vbo,
                           std::optional<FrontFace> face = std::nullopt);

        virtual ~VKGraphicsPipeline();

        virtual void BindPipeline(CommandBuffer *buffer) const override;

        void BindPipeline(CommandBuffer *buffer, bool wireframe, bool receivers = false,
                          bool withRadiance = false) const;

        bool HasVoxelReceiver() const { return receiverPipeline != VK_NULL_HANDLE; }

        virtual void RenderPipeline() const override;

        virtual void UnbindPipeline() const override;

        virtual void RecreatePipeline();

        virtual uint64_t GetHash() override;

        bool drawsTriangleMesh() const override {
            return topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST && GraphicsPipeline::drawsTriangleMesh();
        }

        VkPrimitiveTopology getTopology() const { return topology; }
        bool hasWireframePipeline() const { return wireframePipeline != VK_NULL_HANDLE; }
        VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }

    private:
        void CreatePipeline();

    private:
        VKRenderpass *renderpass;

        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPipeline graphicsPipeline;
        VkPipeline litReceiverPipeline{};
        VkPipeline receiverPipeline{};
        VkPipeline wireframePipeline = VK_NULL_HANDLE;
    };
};

#endif