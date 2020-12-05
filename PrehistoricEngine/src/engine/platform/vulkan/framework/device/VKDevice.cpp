#include "engine/prehistoric/core/util/Includes.hpp"
#include "VKDevice.h"
#include "engine/prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	VKDevice::VKDevice()
	{
		device = VK_NULL_HANDLE;
	}

	VKDevice::~VKDevice()
	{
		device = VK_NULL_HANDLE;
	}

	void VKDevice::CreateLogicalDevice(VKPhysicalDevice* physicalDevice, VkSurfaceKHR surface, const std::vector<const char*>& validationLayers)
	{
		QueueFamilyIndices indices = VKUtil::FindQueueFamilies(physicalDevice->getPhysicalDevice(), surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;
		deviceFeatures.geometryShader = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		std::vector<const char*> ext = physicalDevice->getDeviceExtensions();
		std::vector<const char*> opt = physicalDevice->getOptionalExtensionsSupported();

		for (const auto& extension : opt)
		{
			ext.push_back(extension);
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(ext.size());
		createInfo.ppEnabledExtensionNames = ext.data();

#if defined(PR_VK_ENABLE_VALIDATION_LAYERS)
		if (FrameworkConfig::apiVulkanUseValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}
#else
		createInfo.enabledLayerCount = 0;
#endif

		if (vkCreateDevice(physicalDevice->getPhysicalDevice(), &createInfo, nullptr, &device) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create logical device!");
		}

		graphicsQueue.SetupQueue(device, indices.graphicsFamily);
		presentQueue.SetupQueue(device, indices.presentFamily);
	}

	void VKDevice::DestroyLogicalDevice()
	{
		vkDestroyDevice(device, nullptr);
	}
};