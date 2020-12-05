#ifndef VK_CONTEXT_H
#define VK_CONTEXT_H

#include "prehistoric/common/framework/context/Context.h"

#include <vulkan/vulkan.h>

#include "VKInstance.h"
#include "platform/vulkan/framework/device/VKPhysicalDevice.h"
#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/framework/surface/VKSurface.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"

namespace Prehistoric
{
	class VKContext : public Context
	{
	public:
		VKContext(Window* window);
		virtual ~VKContext();

		VKInstance getInstance() const { return instance; }

		void* getNativePhysicalDevice() override { return &physicalDevice.getPhysicalDevice(); }
		void* getNativeDevice() override { return &logicalDevice.getDevice(); }

		void* getPhysicalDevice() override { return &physicalDevice; }
		void* getDevice() override { return &logicalDevice; }

		VKSurface* getSurface() { return surface.get(); }
	private:
		VKInstance instance;

		VKPhysicalDevice physicalDevice;
		VKDevice logicalDevice;

		std::unique_ptr<VKSurface> surface;
	};
};

#endif