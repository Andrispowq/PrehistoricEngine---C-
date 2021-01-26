#include "Includes.hpp"
#include "VKCommandPool.h"

namespace Prehistoric
{
	VKCommandPool::VKCommandPool(VKDevice* device)
		: device(device)
	{
		QueueFamilyIndices indices = device->getQueueFamilyIndices();

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = indices.graphicsFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

		if (vkCreateCommandPool(device->getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create command pool!\n");
		}
	}

	VKCommandPool::~VKCommandPool()
	{
		buffers.clear();

		vkDestroyCommandPool(device->getDevice(), commandPool, nullptr);
	}

	void VKCommandPool::AddCommandBuffer(VKCommandBuffer& buffer)
	{
		buffers.emplace_back(&buffer);
	}

	void VKCommandPool::AddCommandBuffer()
	{
		buffers.push_back(std::make_unique<VKCommandBuffer>(this, device->getDevice()));
	}

	void VKCommandPool::DeleteCommandBuffer(int index)
	{
		//We erase the element first because accessing deleted memory is not too nice
		auto bufferIndex = std::find(buffers.begin(), buffers.end(), buffers[index]);
		buffers.erase(bufferIndex);
	}

	void VKCommandPool::DeleteCommandBuffers()
	{
		std::vector<VkCommandBuffer> buffs(buffers.size());

		for (size_t i = 0; i < buffers.size(); i++)
		{
			buffs[i] = buffers[i]->getCommandBuffer();
			buffers[i].release(); //The reason for this is that we explicitly delete every command buffer, and not releasing the pointers to the VKCommandBuffers
			//would invoke their destructors, so we would delete them twice
		}

		vkFreeCommandBuffers(device->getDevice(), commandPool, (uint32_t)buffers.size(), buffs.data());
		buffers.clear();
	}

	void VKCommandPool::BindCommandBuffer(int index) const
	{
		buffers[index]->BindBuffer();
	}

	void VKCommandPool::BindCommandPool() const
	{
		for (auto& buffer : buffers)
		{
			buffer->BindBuffer();
		}
	}

	void VKCommandPool::UnbindCommandBuffer(int index) const
	{
		buffers[index]->UnbindBuffer();
	}

	void VKCommandPool::UnbindCommandPool() const
	{
		for (auto& buffer : buffers)
		{
			buffer->UnbindBuffer();
		}
	};
};