#include "Includes.hpp"
#include "VkContext.h"

namespace Prehistoric
{
	VKContext::VKContext(Window* window)
		: Context(window), surface{ nullptr }
	{
		surface = std::make_unique<VKSurface>(window, instance.getInstance());

		physicalDevice.PickPhysicalDevice(instance.getInstance(), surface->getSurface());
		logicalDevice.CreateLogicalDevice(&physicalDevice, surface->getSurface(), instance.getValidationLayers());

		VKUtil::Init(physicalDevice.getPhysicalDevice(), logicalDevice.getDevice());
	}

	VKContext::~VKContext()
	{
		VKUtil::CleanUp(logicalDevice.getDevice());

		logicalDevice.DestroyLogicalDevice();
	}
};
