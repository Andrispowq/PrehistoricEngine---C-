#ifndef VK_FENCE_H
#define VK_FENCE_H

#include <vulkan/vulkan.h>

class VKFence
{
public:
	VKFence(VkDevice device);
	virtual ~VKFence();

	void WaitForFence();
	void ResetFence();

	//Needs to return a reference
	VkFence& getFence() { return fence; }
private:
	VkDevice device;

	VkFence fence;
};

#endif