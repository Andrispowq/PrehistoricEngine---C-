#include "engine/prehistoric/core/util/Includes.hpp"
#include "VKPhysicalDevice.h"
#include "engine/prehistoric/core/config/FrameworkConfig.h"

#include <vulkan/vulkan_beta.h>

namespace Prehistoric
{
	VKPhysicalDevice::VKPhysicalDevice()
	{
		//Swapchain extension -> obligatory
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		//Mesh shading extension
		optionalDeviceExtensions.push_back(VK_NV_MESH_SHADER_EXTENSION_NAME);

		//Ray tracing extensions
		optionalDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		optionalDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

		//Required by VK_KHR_acceleration_structure
		optionalDeviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		optionalDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		optionalDeviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

		//Required for VK_KHR_ray_tracing_pipeline
		optionalDeviceExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);

		//Required by VK_KHR_spirv_1_4
		optionalDeviceExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
	}

	void VKPhysicalDevice::PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			PR_LOG_RUNTIME_ERROR("Failed to find GPUs with Vulkan support!\n");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device, surface, deviceFeatures))
			{
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE)
		{
			PR_LOG_RUNTIME_ERROR("Failed to find a suitable GPU!\n");
		}

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		if (counts & VK_SAMPLE_COUNT_1_BIT) msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		if (counts & VK_SAMPLE_COUNT_2_BIT) msaaSamples = VK_SAMPLE_COUNT_2_BIT;
		if (counts & VK_SAMPLE_COUNT_4_BIT) msaaSamples = VK_SAMPLE_COUNT_4_BIT;
		if (counts & VK_SAMPLE_COUNT_8_BIT) msaaSamples = VK_SAMPLE_COUNT_8_BIT;
		if (counts & VK_SAMPLE_COUNT_16_BIT) msaaSamples = VK_SAMPLE_COUNT_16_BIT;
		if (counts & VK_SAMPLE_COUNT_32_BIT) msaaSamples = VK_SAMPLE_COUNT_32_BIT;
		if (counts & VK_SAMPLE_COUNT_64_BIT) msaaSamples = VK_SAMPLE_COUNT_64_BIT;
	}

	std::vector<const char*> VKPhysicalDevice::getOptionalExtensionsSupported() const
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		std::vector<const char*> supported;

		for (const auto& extension : availableExtensions)
		{
			for (const auto& ext : optionalDeviceExtensions)
			{
				if (ext == extension.extensionName)
				{
					supported.push_back(ext);
				}
			}
		}

		return supported;
	}

	bool VKPhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, VkPhysicalDeviceFeatures features) const
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			QueueFamilyIndices indices = VKUtil::FindQueueFamilies(device, surface);

			bool extensionsSupported = CheckDeviceExtensionSupport(device);

			bool swapChainAdequate = false;
			if (extensionsSupported)
			{
				SwapChainSupportDetails swapChainSupport = VKUtil::QuerySwapChainSupport(device, surface);
				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			}

			return indices.hasGraphicsFamily && indices.hasPresentFamily && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;
		}
		else
		{
			return false;
		}
	}

	bool VKPhysicalDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device) const
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}
};