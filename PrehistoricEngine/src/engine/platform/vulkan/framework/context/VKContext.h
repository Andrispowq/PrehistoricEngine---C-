#ifndef VK_CONTEXT_H
#define VK_CONTEXT_H

#include "prehistoric/common/framework/context/Context.h"

#include <vulkan/vulkan.h>

#include "VKInstance.h"
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

		VKInstance* getInstance() const { return instance.get(); }

		void* getNativePhysicalDevice() override { return &devices->getPhysicalDevice(); }
		void* getNativeDevice() override { return &devices->getDevice(); }

		void* getDevices() override { return devices.get(); }

		VKSurface* getSurface() { return surface.get(); }
	private:
		std::unique_ptr<VKInstance> instance;
		std::unique_ptr<VKDevice> devices;

		std::unique_ptr<VKSurface> surface;
	};
};

#endif