#ifndef VK_COMMAND_BUFFER_H
#define VK_COMMAND_BUFFER_H

#include "prehistoric/common/rendering/command/CommandBuffer.h"

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"

namespace Prehistoric
{
	class VKCommandPool;

	class VKCommandBuffer : public CommandBuffer
	{
	public:
		VKCommandBuffer(VKCommandPool* commandPool, VkDevice device);
		virtual ~VKCommandBuffer() {}

		void BindBuffer() const;
		void UnbindBuffer() const;

		void DeleteBuffer();

		//Needs to return a reference
		VkCommandBuffer& getCommandBuffer() { return commandBuffer; }

		VKCommandBuffer(const VKCommandBuffer&) = default;
	private:
		VKCommandPool* commandPool;
		VkDevice device;

		VkCommandBuffer commandBuffer;
	};
};

#endif