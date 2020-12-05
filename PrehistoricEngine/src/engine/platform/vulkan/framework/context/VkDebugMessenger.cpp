#include "engine/prehistoric/core/util/Includes.hpp"
#include "VkDebugMessenger.h"
#include "engine/platform/Platform.h"

#if defined(PR_ENABLE_DEBUGGING) && defined(PR_VK_ENABLE_VALIDATION_LAYERS)

//Validation layer callbacks:
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::string message;

	message.append("Vulkan validation layer:\n");
	message.append("\tMessage severity: %s\n");
	message.append("\tMessage type: %s\n");
	message.append("\tMessage ID name: %s\n");
	message.append("\tMessage: %s\n");
	
	std::string sSeverity;
	bool hasSeverityBegan = false;

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		if (!hasSeverityBegan)
		{
			hasSeverityBegan = true;
		}
		else
		{
			sSeverity.append("and ");
		}

		sSeverity.append("VERBOSE");
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		if (!hasSeverityBegan)
		{
			hasSeverityBegan = true;
		}
		else
		{
			sSeverity.append("and ");
		}

		sSeverity.append("WARNING");
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		if (!hasSeverityBegan)
		{
			hasSeverityBegan = true;
		}
		else
		{
			sSeverity.append("and ");
		}

		sSeverity.append("ERROR");
	}

	std::string sType;
	bool hasTypeBegan = false;

	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
	{
		if (!hasTypeBegan)
		{
			hasTypeBegan = true;
		}
		else
		{
			sType.append("and ");
		}

		sType.append("PERFORMANCE");
	}
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
	{
		if (!hasTypeBegan)
		{
			hasTypeBegan = true;
		}
		else
		{
			sType.append("and ");
		}

		sType.append("VALIDATION");
	}
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
	{
		if (!hasTypeBegan)
		{
			hasTypeBegan = true;
		}
		else
		{
			sType.append("and ");
		}

		sType.append("GENERAL");
	}

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		PR_LOG_ERROR(message, sSeverity.c_str(), sType.c_str(), pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}
	else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		PR_LOG_WARNING(message, sSeverity.c_str(), sType.c_str(), pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}

	return VK_FALSE;
}

namespace Prehistoric
{
	VkResult CreateDebugUtilsMessengerEXT(VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance& instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	VKDebugMessenger::VKDebugMessenger(VkInstance instance)
		: instance(instance)
	{
		debugMessenger = VK_NULL_HANDLE;
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
		createInfo.pUserData = nullptr; // Optional

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to set up debug messenger!");
		}
	}
	VKDebugMessenger::~VKDebugMessenger()
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	void VKDebugMessenger::CreateMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo)
	{
		messengerCreateInfo = {};
		messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		messengerCreateInfo.pfnUserCallback = DebugCallback;
		//messengerCreateInfo.pUserData = nullptr;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance& instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
};

#endif