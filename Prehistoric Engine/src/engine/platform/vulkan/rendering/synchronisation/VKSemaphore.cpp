#include "engine/prehistoric/core/util/Includes.hpp"
#include "VKSemaphore.h"

VKSemaphore::VKSemaphore(VkDevice device)
	: device(device)
{
	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(device, &info, nullptr, &semaphore) != VK_SUCCESS)
	{
		PR_LOG_RUNTIME_ERROR("Failed to create semaphore!\n");
	}
}

VKSemaphore::~VKSemaphore()
{
	vkDestroySemaphore(device, semaphore, nullptr);
}