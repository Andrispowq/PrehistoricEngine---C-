#ifndef VK_DEBUG_MESSENGER_H
#define VK_DEBUG_MESSENGER_H

#include <vulkan/vulkan.h>

namespace Prehistoric
{
	class VKDebugMessenger
	{
	public:
		VKDebugMessenger(VkInstance instnace);
		virtual ~VKDebugMessenger();

		static void CreateMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo);

		VkDebugUtilsMessengerEXT getDebugMessenger() const { return debugMessenger; }
	private:
		VkInstance instance;

		VkDebugUtilsMessengerEXT debugMessenger;
	};
};

#endif