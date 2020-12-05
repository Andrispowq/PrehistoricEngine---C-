#include "Includes.hpp"
#include "VKQueue.h"

namespace Prehistoric
{
	void VKQueue::SetupQueue(VkDevice device, uint32_t family)
	{
		vkGetDeviceQueue(device, family, 0, &queue);
	}
};