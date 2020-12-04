#include "engine/prehistoric/core/util/Includes.hpp"
#include "VKQueue.h"

void VKQueue::SetupQueue(VkDevice device, uint32_t family)
{
	vkGetDeviceQueue(device, family, 0, &queue);
}