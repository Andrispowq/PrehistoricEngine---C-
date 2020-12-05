#ifndef VK_SEMAPHORE_H
#define VK_SEMAPHORE_H

#include <vulkan/vulkan.h>

namespace Prehistoric
{
	class VKSemaphore
	{
	public:
		VKSemaphore(VkDevice device);
		virtual ~VKSemaphore();

		//Needs to return a reference
		VkSemaphore& getSemaphore() { return semaphore; }
	private:
		VkDevice device;

		VkSemaphore semaphore;
	};
};

#endif