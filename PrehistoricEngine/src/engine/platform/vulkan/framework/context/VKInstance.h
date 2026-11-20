#ifndef VK_INSTANCE_H
#define VK_INSTANCE_H

//GLFW supplies the surface extensions required by the current platform.
#include <vulkan/vulkan.h>
#define PR_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VKDebugMessenger.h"

namespace Prehistoric
{
	class VKInstance
	{
	public:
		VKInstance();
		virtual ~VKInstance();

		VkInstance getInstance() const { return instance; }
		VKDebugMessenger* getDebugMessenger() const { return messenger.get(); }

		std::vector<const char*> getValidationLayers() const { return validationLayers; }
	private:
		bool CheckValidationLayerSupport() const;
		std::vector<const char*> GetRequiredExtensions() const;

		void ListExtensions() const;
	private:
		VkInstance instance = VK_NULL_HANDLE;
		std::unique_ptr<VKDebugMessenger> messenger; //Owned pointer, but optional

		std::vector<const char*> validationLayers;
	};
};

#endif