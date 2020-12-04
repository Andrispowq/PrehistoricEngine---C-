#ifndef VK_DEVICE_SELECTOR
#define VK_DEVICE_SELECTOR

#include <vulkan/vulkan.h>

#include "VKPhysicalDevice.h"
#include "engine/platform/vulkan/framework/surface/VKSurface.h"
#include "engine/platform/vulkan/framework/queue/VKQueue.h"

class VKDevice
{
public:
	VKDevice();
	virtual ~VKDevice();

	void CreateLogicalDevice(VKPhysicalDevice* physicalDevice, VkSurfaceKHR surface, const std::vector<const char*>& validationLayers);
	void DestroyLogicalDevice();

	//This needs to return a reference
	VkDevice& getDevice() { return device; }

	VKQueue& getGraphicsQueue() { return graphicsQueue; }
	VKQueue& getPresentQueue() { return presentQueue; }
private:
	VkDevice device;

	VKQueue graphicsQueue;
	VKQueue presentQueue;
};

#endif