#include "Includes.hpp"
#include "VKInstance.h"

#include <cstring>
#include <stdexcept>

#include "platform/Platform.h"

#include "prehistoric/application/Application.h"

#define VK_LAYER_KHRONOS_VALIDATION_EXTENSION_NAME "VK_LAYER_KHRONOS_validation"

namespace Prehistoric
{
	VKInstance::VKInstance()
		: messenger{ nullptr }
	{
		//Check for validation layer support, but we only use it in debug mode
#if defined(PR_ENABLE_DEBUGGING) && defined(PR_VK_ENABLE_VALIDATION_LAYERS)
		if (__FrameworkConfig.apiVulkanUseValidationLayers)
		{
			validationLayers.push_back(VK_LAYER_KHRONOS_VALIDATION_EXTENSION_NAME);

			if (!CheckValidationLayerSupport())
			{
				PR_LOG_RUNTIME_ERROR("Validation layers were requested, but they are not available!");
			}
		}
#endif

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = __FrameworkConfig.windowName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(__FrameworkConfig.apiVersion.x, __FrameworkConfig.apiVersion.y, 0);
		appInfo.pEngineName = __FrameworkConfig.windowName.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(__FrameworkConfig.apiVersion.x, __FrameworkConfig.apiVersion.y, 0);
		appInfo.apiVersion = VK_MAKE_VERSION(__FrameworkConfig.apiVersion.x, __FrameworkConfig.apiVersion.y, 0);

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		//Extensions
		auto extensions = GetRequiredExtensions();

#if defined(PR_PLATFORM_MACOS)
		for (const auto& extension : extensions)
		{
			if (strcmp(extension, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0)
				createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		}
#endif

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

#if defined(PR_ENABLE_DEBUGGING) && defined(PR_VK_ENABLE_VALIDATION_LAYERS)
		//This validation layer is only used for the instance creation and destruction
		VkDebugUtilsMessengerCreateInfoEXT createMessenger;
#endif
		if (__FrameworkConfig.apiVulkanUseValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

#if defined(PR_ENABLE_DEBUGGING) && defined(PR_VK_ENABLE_VALIDATION_LAYERS)
			VKDebugMessenger::CreateMessengerCreateInfo(createMessenger);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&createMessenger;
#else
			createInfo.pNext = nullptr;
#endif
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		ListExtensions();

		//The instance creation
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan instance!");
		}

#if defined(PR_ENABLE_DEBUGGING) && defined(PR_VK_ENABLE_VALIDATION_LAYERS)
		if (__FrameworkConfig.apiVulkanUseValidationLayers)
		{
			messenger = std::make_unique<VKDebugMessenger>(instance);
		}
#endif
	}

	VKInstance::~VKInstance()
	{
#if defined(PR_ENABLE_DEBUGGING) && defined(PR_VK_ENABLE_VALIDATION_LAYERS)
		if (__FrameworkConfig.apiVulkanUseValidationLayers)
		{
			delete messenger.release();
		}
#endif

		//The instance destruction
		vkDestroyInstance(instance, nullptr);
	}

	void VKInstance::ListExtensions() const
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		PR_LOG_MESSAGE("Available extensions:\n");

		for (const auto& extension : extensions)
		{
			PR_LOG_MESSAGE("\t%s\n", extension.extensionName);
		}
		PR_LOG_MESSAGE("\n");
	}

	bool VKInstance::CheckValidationLayerSupport() const
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> VKInstance::GetRequiredExtensions() const
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		if (!glfwExtensions || glfwExtensionCount == 0)
			throw std::runtime_error("GLFW could not find Vulkan surface extensions!");

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#if defined(PR_PLATFORM_MACOS)
		uint32_t extensionCount = 0;
		if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS)
			throw std::runtime_error("Failed to enumerate Vulkan instance extensions!");
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to enumerate Vulkan instance extensions!");

		for (const auto& extension : availableExtensions)
		{
			if (strcmp(extension.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0)
			{
				extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
				break;
			}
		}
#endif

		if (__FrameworkConfig.apiVulkanUseValidationLayers)
		{
			//This is required for validation layers
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}
};