#ifndef VK_DEVICE_SELECTOR
#define VK_DEVICE_SELECTOR

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/surface/VKSurface.h"
#include "platform/vulkan/framework/queue/VKQueue.h"

namespace Prehistoric
{
	struct QueueFamilyIndices
	{
		uint32_t graphicsFamily;
		uint32_t computeFamily;
		uint32_t presentFamily;

		bool hasGraphicsFamily;
		bool hasComputeFamily;
		bool hasPresentFamily;
	};

	class VKDevice
	{
	public:
		VKDevice(VkInstance instance, VKSurface* surface, const std::vector<const char*>& validationLayers);
		virtual ~VKDevice();

		QueueFamilyIndices getQueueFamilyIndices() const { return indices; };

		//These need to return a reference
		VkPhysicalDevice& getPhysicalDevice() { return physicalDevice; }
		VkDevice& getDevice() { return device; }

		VKQueue& getGraphicsQueue() { return *graphicsQueue; }
		VKQueue& getComputeQueue() { return *computeQueue; }
		VKQueue& getPresentQueue() { return *presentQueue; }

		std::vector<const char*> getDeviceExtensions() { return deviceExtensions; }
		std::vector<const char*> getOptionalExtensionsSupported() const;

		VkSampleCountFlagBits getSampleCount() const { return msaaSamples; }

	private:
		//Utility functions for picking GPU
		bool IsDeviceSuitable(VkPhysicalDevice physDev, VkPhysicalDeviceFeatures features);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice physDev) const;

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physDev = VK_NULL_HANDLE);

	private:
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		QueueFamilyIndices indices;

		VKSurface* _surface;

		std::unique_ptr<VKQueue> graphicsQueue;
		std::unique_ptr<VKQueue> computeQueue;
		std::unique_ptr<VKQueue> presentQueue;

		std::vector<const char*> deviceExtensions;
		std::vector<const char*> optionalDeviceExtensions;

		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	};
};

#endif