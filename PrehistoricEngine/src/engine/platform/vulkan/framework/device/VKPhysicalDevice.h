#ifndef VK_PHYSICAL_DEVICE_SELECTOR
#define VK_PHYSICAL_DEVICE_SELECTOR

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/surface/VKSurface.h"
#include "platform/vulkan/framework/context/VKInstance.h"

#include "platform/vulkan/util/VKUtil.h"

namespace Prehistoric
{
	class VKPhysicalDevice
	{
	public:
		VKPhysicalDevice();
		virtual ~VKPhysicalDevice() {}

		void PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

		//This needs to return a reference because the memory address of this variable is taken
		VkPhysicalDevice& getPhysicalDevice() { return physicalDevice; }
		std::vector<const char*>& getDeviceExtensions() { return deviceExtensions; }

		std::vector<const char*> getOptionalExtensionsSupported() const;

		VkSampleCountFlagBits getSampleCount() const { return msaaSamples; }
	private:
		//Utility functions for picking GPU
		bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, VkPhysicalDeviceFeatures features) const;
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
	private:
		VkPhysicalDevice physicalDevice;

		std::vector<const char*> deviceExtensions;
		std::vector<const char*> optionalDeviceExtensions;
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	};
};

#endif