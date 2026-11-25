#ifndef VK_MESH_VERTEX_BUFFER_H
#define VK_MESH_VERTEX_BUFFER_H

#include "prehistoric/common/buffer/MeshVertexBuffer.h"

#include <vulkan/vulkan.h>

#include "VKBuffer.h"

#include <vector>

#include "platform/vulkan/rendering/command/VKCommandBuffer.h"
#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"

namespace Prehistoric {
    class VKMeshVertexBuffer : public MeshVertexBuffer {
    public:
        VKMeshVertexBuffer(Window *window, const Model &model);

        virtual ~VKMeshVertexBuffer() {
        }

        void setInstanceCount(uint32_t count) { instanceCount = count; }

        void Bind(CommandBuffer *commandBuffer) const override;

        VkBuffer GetVertexBuffer() const { return vertexBuffer->getBuffer(); }
        VkBuffer GetIndexBuffer() const { return indexBuffer->getBuffer(); }
        uint32_t FirstIndex(size_t submesh) const { return static_cast<uint32_t>(index_offsets[submesh]); }
        uint32_t IndexCount(size_t submesh) const { return static_cast<uint32_t>(sizes[submesh]); }

        void DrawAll(VKCommandBuffer *buffer) const;

        void Draw(CommandBuffer *commandBuffer, uint32_t submesh) const override;

        void Unbind() const override {
        }

        VkVertexInputBindingDescription getBindingDescription() const;

        std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() const;

    private:
        std::unique_ptr<VKBuffer> vertexBuffer;
        std::unique_ptr<VKBuffer> indexBuffer;

        uint32_t instanceCount = 1;
        size_t totalVertices = 0;
        VKDevice *device;
        VKSwapchain *swapchain;
    };
};

#endif