#ifndef VK_COMMAND_POOL_H
#define VK_COMMAND_POOL_H

#include <vulkan/vulkan.h>

#include "VKCommandBuffer.h"
#include "platform/vulkan/util/VKUtil.h"

namespace Prehistoric
{
	class VKCommandPool
	{
	public:
		VKCommandPool(VKDevice* device);
		virtual ~VKCommandPool();

		void AddCommandBuffer(VKCommandBuffer& buffer);
		void AddCommandBuffer();

		void DeleteCommandBuffer(int index);
		void DeleteCommandBuffers();

		void BindCommandBuffer(int index) const;
		void BindCommandPool() const;

		void UnbindCommandBuffer(int index) const;
		void UnbindCommandPool() const;

		VkCommandPool getCommandPool() const { return commandPool; }

		VKCommandBuffer* getCommandBuffer(int index) const { return buffers[index].get(); }
		std::vector<std::unique_ptr<VKCommandBuffer>>& getCommandBuffers() { return buffers; }
	private:
		VKDevice* device;

		VkCommandPool commandPool;
		std::vector<std::unique_ptr<VKCommandBuffer>> buffers;
	};
};

#endif