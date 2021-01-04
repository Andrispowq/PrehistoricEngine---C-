#include "Includes.hpp"
#include "VkPipeline.h"

#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"
#include "platform/vulkan/framework/context/VKContext.h"
#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	VKPipeline::VKPipeline(Window* window, ResourceStorage* resourceStorage, ShaderHandle shader)
		: Pipeline(window, resourceStorage, shader)
	{
		VKSwapchain* swapchain = static_cast<VKSwapchain*>(window->getSwapchain());
		VKContext* context = static_cast<VKContext*>(window->getContext());

		this->surface = context->getSurface();
		this->physicalDevice = (VKPhysicalDevice*)context->getPhysicalDevice();
		this->device = (VKDevice*)context->getDevice();

		this->window = window;
		this->swapchain = swapchain;
	}
};