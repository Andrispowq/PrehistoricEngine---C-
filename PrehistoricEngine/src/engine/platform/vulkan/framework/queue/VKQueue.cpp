#include "Includes.hpp"
#include "VKQueue.h"

namespace Prehistoric
{
	VKQueue::VKQueue(VkDevice device, uint32_t family)
	{
		vkGetDeviceQueue(device, family, 0, &queue);
	}

	VKQueue::~VKQueue()
	{
	}
};