#ifndef VK_COMMAND_BUFFER_H
#define VK_COMMAND_BUFFER_H

#include "engine/prehistoric/common/rendering/command/CommandBuffer.h"

#include <vulkan/vulkan.h>

#include "engine/platform/vulkan/framework/device/VKDevice.h"

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

#endif