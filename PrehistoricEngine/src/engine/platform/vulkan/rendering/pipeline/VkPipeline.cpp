#include "Includes.hpp"
#include "VkPipeline.h"

#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"
#include "platform/vulkan/framework/context/VKContext.h"
#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	VKPipeline::VKPipeline(Window* window, AssetManager* manager, ShaderHandle shader)
		: Pipeline(window, manager, shader)
	{
		VKSwapchain* swapchain = static_cast<VKSwapchain*>(window->getSwapchain());
		VKContext* context = static_cast<VKContext*>(window->getContext());

		this->surface = context->getSurface();
		this->device = (VKDevice*)context->getDevices();

		this->window = window;
		this->swapchain = swapchain;
	}
};