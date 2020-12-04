#include "engine/prehistoric/core/util/Includes.hpp"
#include "VKFence.h"

VKFence::VKFence(VkDevice device)
	: device(device)
{
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateFence(device, &info, nullptr, &fence) != VK_SUCCESS)
	{
		PR_LOG_RUNTIME_ERROR("Failed to create fence!\n");
	}
}

VKFence::~VKFence()
{
	vkDestroyFence(device, fence, nullptr);
}

void VKFence::WaitForFence()
{
	vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
}

void VKFence::ResetFence()
{
	vkResetFences(device, 1, &fence);
}