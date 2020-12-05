#include "Includes.hpp"
#include <glew.h>
#include "VkPipeline.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"
#include "platform/vulkan/framework/context/VKContext.h"

namespace Prehistoric
{
	VKPipeline::VKPipeline(Window* window, AssetManager* manager, size_t shaderID)
		: Pipeline(window, manager, shaderID)
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