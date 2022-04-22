#include "Includes.hpp"
#include "VKDevice.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	VKDevice::VKDevice(VkInstance instance, VKSurface* surface, const std::vector<const char*>& validationLayers)
		: graphicsQueue{ nullptr }, presentQueue{ nullptr }, _surface(surface)
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

		//Pick a physical device:
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
			if (IsDeviceSuitable(device, deviceFeatures))
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

		//Create the logical device:
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

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

		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;
		deviceFeatures.geometryShader = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		std::vector<const char*> ext = deviceExtensions;
		std::vector<const char*> opt = getOptionalExtensionsSupported();

		for (const auto& extension : opt)
		{
			ext.push_back(extension);
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(ext.size());
		createInfo.ppEnabledExtensionNames = ext.data();

#if defined(PR_VK_ENABLE_VALIDATION_LAYERS)
		if (__FrameworkConfig.apiVulkanUseValidationLayers)
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

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create logical device!\n");
		}

		graphicsQueue = std::make_unique<VKQueue>(device, indices.graphicsFamily);
		computeQueue = std::make_unique<VKQueue>(device, indices.computeFamily);
		presentQueue = std::make_unique<VKQueue>(device, indices.presentFamily);
	}

	VKDevice::~VKDevice()
	{
		vkDestroyDevice(device, nullptr);
	}

	std::vector<const char*> VKDevice::getOptionalExtensionsSupported() const
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

	bool VKDevice::IsDeviceSuitable(VkPhysicalDevice physDev, VkPhysicalDeviceFeatures features)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		vkGetPhysicalDeviceProperties(physDev, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physDev, &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			QueueFamilyIndices indices = FindQueueFamilies(physDev);

			bool extensionsSupported = CheckDeviceExtensionSupport(physDev);

			bool swapChainAdequate = false;
			if (extensionsSupported)
			{
				SwapchainSupportDetails swapChainSupport = _surface->QuerySwapChainSupport(physDev);
				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			}

			return indices.hasGraphicsFamily && indices.hasPresentFamily && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;
		}
		else
		{
			return false;
		}
	}

	bool VKDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device) const
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

	QueueFamilyIndices VKDevice::FindQueueFamilies(VkPhysicalDevice device)
	{
		if (device == VK_NULL_HANDLE)
			device = physicalDevice;

		indices = { 0, false, 0, false };

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;

		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
				indices.hasGraphicsFamily = true;
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.computeFamily = i;
				indices.hasComputeFamily = true;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface->getSurface(), &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
				indices.hasPresentFamily = true;
			}

			if (indices.hasGraphicsFamily && indices.hasPresentFamily && indices.hasComputeFamily)
			{
				break;
			}

			i++;
		}

		return indices;
	}
};