#include "Includes.hpp"
#include "VKCapabilities.h"
#include "platform/vulkan/framework/device/VKPhysicalDevice.h"

#include <vulkan/vulkan_beta.h>

namespace Prehistoric
{
	void VKCapabilities::QueryCapabilities(void* physicalDevice)
	{
		VKPhysicalDevice* physicalDev = reinterpret_cast<VKPhysicalDevice*>(physicalDevice);

		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceMemoryProperties memoryProps;

		vkGetPhysicalDeviceProperties(physicalDev->getPhysicalDevice(), &properties);
		vkGetPhysicalDeviceFeatures(physicalDev->getPhysicalDevice(), &deviceFeatures);
		vkGetPhysicalDeviceMemoryProperties(physicalDev->getPhysicalDevice(), &memoryProps);

		//Getting extension features:
	/*
		VkPhysicalDeviceRayTracingPropertiesNV rtProps;
		rtProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
		rtProps.pNext = nullptr;

		VkPhysicalDeviceProperties2 props;
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		props.pNext = &rtProps;

		vkGetPhysicalDeviceProperties2(physicalDev->getPhysicalDevice(), &props);
	*/

		physicalDeviceCaps.name = properties.deviceName;
		physicalDeviceCaps.discrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

		driverCaps.driverVersionMajor = VK_VERSION_MAJOR(properties.driverVersion);
		driverCaps.driverVersionMinor = VK_VERSION_MINOR(properties.driverVersion);
		driverCaps.driverVersionPatch = VK_VERSION_PATCH(properties.driverVersion);

		driverCaps.apiVersionMajor = VK_VERSION_MAJOR(properties.apiVersion);
		driverCaps.apiVersionMinor = VK_VERSION_MINOR(properties.apiVersion);
		driverCaps.apiVersionPatch = VK_VERSION_PATCH(properties.apiVersion);

		memoryCaps.vramSize = (uint64_t)(memoryProps.memoryHeaps[0].size / 1024.0f);

		VkDeviceSize sharedSize = 0;

		for (size_t i = 1; i < memoryProps.memoryHeapCount; i++)
		{
			sharedSize += memoryProps.memoryHeaps[i].size;
		}

		memoryCaps.sharedRamSize = (uint64_t)(sharedSize / 1024.0f);

		shaderCaps.geometryShaderSupported = deviceFeatures.geometryShader;
		shaderCaps.tessellationShaderSupported = deviceFeatures.tessellationShader;

		shaderCaps.computeShaderSupported = false;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDev->getPhysicalDevice(), &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDev->getPhysicalDevice(), &queueFamilyCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				shaderCaps.computeShaderSupported = true;
				break;
			}
		}

		shaderCaps.shaderVersion = 1;

		shaderCaps.rayTracingSupported = false;

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDev->getPhysicalDevice(), nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDev->getPhysicalDevice(), nullptr, &extensionCount, availableExtensions.data());

		bool rtPipSupp = false;
		bool accStrSupp = false;
		for (uint32_t i = 0; i < extensionCount; i++)
		{
			if (strcmp(availableExtensions[i].extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0)
			{
				rtPipSupp = true;
			}
			else if (strcmp(availableExtensions[i].extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0)
			{
				accStrSupp = true;
			}

			if (rtPipSupp && accStrSupp)
			{
				shaderCaps.rayTracingSupported = true;
				break;
			}
		}

		shaderCaps.meshShadingSupported = false;

		for (uint32_t i = 0; i < extensionCount; i++)
		{
			if (strcmp(availableExtensions[i].extensionName, VK_NV_MESH_SHADER_EXTENSION_NAME) == 0)
			{
				shaderCaps.meshShadingSupported = true;
				break;
			}
		}

		limits.maxTextureResolution = properties.limits.maxImageDimension2D;
		limits.maxTextureSlots = 32;
	}
};