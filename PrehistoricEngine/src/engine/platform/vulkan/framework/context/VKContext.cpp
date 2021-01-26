#include "Includes.hpp"
#include "VkContext.h"

namespace Prehistoric
{
	VKContext::VKContext(Window* window)
		: Context(window), devices{ nullptr }, surface{ nullptr }
	{
		instance = std::make_unique<VKInstance>();

		surface = std::make_unique<VKSurface>(window, instance->getInstance());
		devices = std::make_unique<VKDevice>(instance->getInstance(), surface.get(), instance->getValidationLayers());

		VKUtil::Init(devices.get());
	}

	VKContext::~VKContext()
	{
		VKUtil::CleanUp(devices.get());

		delete devices.release();
		delete surface.release();
	}
};
