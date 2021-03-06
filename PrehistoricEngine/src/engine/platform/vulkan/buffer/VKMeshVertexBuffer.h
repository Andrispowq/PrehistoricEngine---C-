#ifndef VK_MESH_VERTEX_BUFFER_H
#define VK_MESH_VERTEX_BUFFER_H

#include "prehistoric/common/buffer/MeshVertexBuffer.h"

#include <vulkan/vulkan.h>

#include "VKBuffer.h"

#include <vector>

#include "platform/vulkan/rendering/command/VKCommandBuffer.h"
#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"

namespace Prehistoric
{
	class VKMeshVertexBuffer : public MeshVertexBuffer
	{
	public:
		VKMeshVertexBuffer(Window* window, const Mesh& mesh);
		virtual ~VKMeshVertexBuffer() {}

		void Bind(CommandBuffer* commandBuffer) const override;
		void Draw(CommandBuffer* commandBuffer) const override;
		void Unbind() const override {}

		VkVertexInputBindingDescription getBindingDescription() const;
		std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() const;
	private:
		std::unique_ptr<VKBuffer> vertexBuffer;
		std::unique_ptr<VKBuffer> indexBuffer;

		VKDevice* device;
		VKSwapchain* swapchain;
	};
};

#endif