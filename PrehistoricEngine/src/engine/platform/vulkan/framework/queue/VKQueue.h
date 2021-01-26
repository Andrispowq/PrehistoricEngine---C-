#ifndef VK_QUEUE_MANAGER_H
#define VK_QUEUE_MANAGER_H

#include <vulkan/vulkan.h>

namespace Prehistoric
{
	class VKQueue
	{
	public:
		VKQueue(VkDevice device, uint32_t family);
		~VKQueue();

		VkQueue getQueue() const { return queue; }
	private:
		VkQueue queue;
	};
};

#endif;