#include "Includes.hpp"
#include <glad/glad.h>
#include "VKMeshVertexBuffer.h"
#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

#include "prehistoric/core/engines/RenderingEngine.h"

namespace Prehistoric
{
	VKMeshVertexBuffer::VKMeshVertexBuffer(Window* window, const Model& model)
		: MeshVertexBuffer(window, model)
	{
		this->device = reinterpret_cast<VKDevice*>(window->getContext()->getDevices());
		this->swapchain = (VKSwapchain*)window->getSwapchain();

		std::vector<float> vdata;
		std::vector<uint32_t> idata;

		vertices.resize(submeshCount);
		index_offsets.resize(submeshCount);
		sizes.resize(submeshCount);

		size_t vertCount = 0;
		size_t index = 0;
		for (auto& mesh : model.getMeshes())
		{
			std::vector<float> vdat = mesh.GetVertexData();
			std::vector<uint32_t> idat = mesh.GetIndexData();

			vdata.insert(vdata.begin(), vdat.begin(), vdat.end());

			for (uint32_t& elem : idat)
			{
				elem += size;
			}

			size_t verts = mesh.getVertices().size();
			size_t inds = mesh.getIndices().size();

			vertices[index] = verts;
			index_offsets[index] = size;
			sizes[index] = inds;

			size += inds;
			index++;
		}

		//Building mesh data
		VkDeviceSize vBufferSize = vertCount * Vertex::getSize();
		VkDeviceSize iBufferSize = size * sizeof(uint32_t);

		vertexBuffer = std::make_unique<VKBuffer>(device, vBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		indexBuffer = std::make_unique<VKBuffer>(device, iBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Creation of the vertex buffer
		std::unique_ptr<VKBuffer> stagingBuffer = std::make_unique<VKBuffer>(device, vBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer->MapMemory();
		stagingBuffer->Store(vdata.data());
		stagingBuffer->UnmapMemory();

		vertexBuffer->Copy(stagingBuffer.get());

		//Creation of index buffer
		stagingBuffer = std::make_unique<VKBuffer>(device, iBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer->MapMemory();
		stagingBuffer->Store(idata.data());
		stagingBuffer->UnmapMemory();

		indexBuffer->Copy(stagingBuffer.get());
	}

	void VKMeshVertexBuffer::Bind(CommandBuffer* commandBuffer) const
	{
		VKCommandBuffer* buffer = (VKCommandBuffer*)commandBuffer;

		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(buffer->getCommandBuffer(), 0, 1, &vertexBuffer->getBuffer(), offsets);
		vkCmdBindIndexBuffer(buffer->getCommandBuffer(), indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void VKMeshVertexBuffer::Draw(CommandBuffer* commandBuffer, uint32_t submesh) const
	{
		if (submesh >= submeshCount)
		{
			return;
		}

		RenderingEngine::getStats().drawcalls += 1;
		RenderingEngine::getStats().vertexCount += vertices[submesh];
		RenderingEngine::getStats().indexCount += sizes[submesh];

		vkCmdDrawIndexed(((VKCommandBuffer*)commandBuffer)->getCommandBuffer(), sizes[submesh], 1, index_offsets[submesh], 0, 0);
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