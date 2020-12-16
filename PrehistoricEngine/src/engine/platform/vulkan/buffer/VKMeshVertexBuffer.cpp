#include "Includes.hpp"
#include <glad/glad.h>
#include "VKMeshVertexBuffer.h"
#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

namespace Prehistoric
{
	VKMeshVertexBuffer::VKMeshVertexBuffer(const Mesh& mesh, Window* window)
		: MeshVertexBuffer(mesh), vertexBuffer(nullptr), indexBuffer(nullptr)
	{
		this->physicalDevice = reinterpret_cast<VKPhysicalDevice*>(window->getContext()->getPhysicalDevice());
		this->device = reinterpret_cast<VKDevice*>(window->getContext()->getDevice());

		this->swapchain = (VKSwapchain*)window->getSwapchain();

		this->size = (uint32_t)mesh.getIndices().size();

		//Building mesh data
		VkDeviceSize vBufferSize = mesh.getVertices().size() * Vertex::getSize();
		VkDeviceSize iBufferSize = mesh.getIndices().size() * sizeof(uint16_t);

		this->vertexBuffer = std::make_unique<VKBuffer>(physicalDevice, device, vBufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		this->indexBuffer = std::make_unique<VKBuffer>(physicalDevice, device, iBufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		std::vector<float> vData = mesh.GetVertexData();
		std::vector<uint16_t> iData = mesh.GetIndexData();

		//Creation of the vertex buffer
		std::unique_ptr<VKBuffer> stagingBuffer = std::make_unique<VKBuffer>(physicalDevice, device, vBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer->MapMemory();
		stagingBuffer->Store(vData.data());
		stagingBuffer->UnmapMemory();

		vertexBuffer->Copy(stagingBuffer.get());

		//Creation of index buffer
		stagingBuffer = std::make_unique<VKBuffer>(physicalDevice, device, iBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer->MapMemory();
		stagingBuffer->Store(iData.data());
		stagingBuffer->UnmapMemory();

		indexBuffer->Copy(stagingBuffer.get());
	}

	void VKMeshVertexBuffer::Bind(CommandBuffer* commandBuffer) const
	{
		VKCommandBuffer* buffer = (VKCommandBuffer*)commandBuffer;

		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(buffer->getCommandBuffer(), 0, 1, &vertexBuffer->getBuffer(), offsets);
		vkCmdBindIndexBuffer(buffer->getCommandBuffer(), indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16);
	}

	void VKMeshVertexBuffer::Draw(CommandBuffer* commandBuffer) const
	{
		vkCmdDrawIndexed(((VKCommandBuffer*)commandBuffer)->getCommandBuffer(), size, 1, 0, 0, 0);
	}

	VkVertexInputBindingDescription VKMeshVertexBuffer::getBindingDescription() const
	{
		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = (uint32_t)Vertex::getSize();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription> VKMeshVertexBuffer::getAttributeDescriptions() const
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = 0;

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = uint32_t(Vector3f::size());

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = uint32_t(Vector3f::size() + Vector2f::size());

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = uint32_t(Vector3f::size() + Vector2f::size() + Vector3f::size());

		return attributeDescriptions;
	}
};