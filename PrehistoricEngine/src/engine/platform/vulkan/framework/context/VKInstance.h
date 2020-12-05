#ifndef VK_INSTANCE_H
#define VK_INSTANCE_H

//GLFW is only included for getting the VK_KHR_win32_surface extensions for windows
#include <vulkan/vulkan.h>
#define PR_INCLUDE_VULKAN
#include <glfw3.h>

#include "VKDebugMessenger.h"
#include "prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	class VKInstance
	{
	public:
		VKInstance();
		virtual ~VKInstance();

		VkInstance getInstance() const { return instance; }
		VKDebugMessenger& getDebugMessenger() { return *messenger; }

		std::vector<const char*> getValidationLayers() const { return validationLayers; }
	private:
		bool CheckValidationLayerSupport() const;
		std::vector<const char*> GetRequiredExtensions() const;

		void ListExtensions() const;
	private:
		VkInstance instance;
		VKDebugMessenger* messenger; //Owned pointer, but optional

		std::vector<const char*> validationLayers;
	};
};

#endif