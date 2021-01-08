#include "Includes.hpp"
#include "VKRenderer.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"

#include "platform/vulkan/framework/context/VKContext.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"

#include "prehistoric/core/resources/AssetManager.h"

namespace Prehistoric
{
	VKRenderer::VKRenderer(Window* window, Camera* camera, AssembledAssetManager* manager)
		: Renderer(window, camera, manager), commandPool(nullptr), renderpass(nullptr)
	{
		VKContext* context = (VKContext*)window->getContext();
		VKSwapchain* swapchain = (VKSwapchain*)window->getSwapchain();

		VKPhysicalDevice* physicalDevice = (VKPhysicalDevice*)context->getPhysicalDevice();
		VKDevice* device = (VKDevice*)context->getDevice();

		renderpass = std::make_unique<VKRenderpass>(physicalDevice, device->getDevice(), swapchain->getSwapchainImageFormat());

		size_t NumImages = swapchain->getSwapchainImageViews().size();
		primaryFramebuffers.resize(NumImages);

		for (size_t i = 0; i < NumImages; i++)
		{
			VkImageView attachments[] =
			{
				swapchain->getColourImageView(),
				swapchain->getDepthImageView(),
				swapchain->getSwapchainImageViews()[i]
			};

			primaryFramebuffers[i] = std::make_unique<VKFramebuffer>(device->getDevice(), renderpass->getRenderPass(), swapchain->getSwapchainExtent().width,
				swapchain->getSwapchainExtent().height, 3, attachments);
		}

		//Create command pool
		commandPool = std::make_unique<VKCommandPool>(physicalDevice->getPhysicalDevice(), device->getDevice(), context->getSurface());

		for (size_t i = 0; i < NumImages; i++)
		{
			commandPool->AddCommandBuffer();
		}

		swapchain->setRenderpass(renderpass.get()); //A nastly little cheat to avoid modifying everything in the Pipeline system because the Vulkan Graphics Pipelines require the renderpass
	}

	void VKRenderer::PrepareRendering()
	{
		VKSwapchain* swapchain = (VKSwapchain*)window->getSwapchain();
		swapchain->GetNextImageIndex();

		if (window->isResized())
		{
			delete renderpass.release();
			primaryFramebuffers.clear();

			uint32_t width = window->getWidth();
			uint32_t height = window->getHeight();

			camera->SetProjection(camera->getFov(), (float)width, (float)height);
			swapchain->SetWindowSize(width, height);

			VKContext* context = (VKContext*)window->getContext();
			VKSwapchain* swapchain = (VKSwapchain*)window->getSwapchain();

			VKPhysicalDevice* physicalDevice = (VKPhysicalDevice*)context->getPhysicalDevice();
			VKDevice* device = (VKDevice*)context->getDevice();

			renderpass = std::make_unique<VKRenderpass>(physicalDevice, device->getDevice(), swapchain->getSwapchainImageFormat());
			swapchain->setRenderpass(renderpass.get());

			size_t NumImages = swapchain->getSwapchainImageViews().size();
			primaryFramebuffers.resize(NumImages);
			for (size_t i = 0; i < NumImages; i++)
			{
				VkImageView attachments[] =
				{
					swapchain->getColourImageView(),
					swapchain->getDepthImageView(),
					swapchain->getSwapchainImageViews()[i]
				};

				primaryFramebuffers[i] = std::make_unique<VKFramebuffer>(device->getDevice(), renderpass->getRenderPass(), swapchain->getSwapchainExtent().width,
					swapchain->getSwapchainExtent().height, 3, attachments);
			}

			//Create command pool
			commandPool->DeleteCommandBuffers();
			for (size_t i = 0; i < NumImages; i++)
			{
				commandPool->AddCommandBuffer();
			}

			//Recreate the pipelines
			std::vector<Pipeline*> pipes = manager->get<Pipeline>();
			for (Pipeline* pipe : pipes)
			{
				pipe->setViewportSize({ (float)width, (float)height });
				pipe->setScissorSize({ width, height });
				pipe->RecreatePipeline();
			}

			window->setResized(false);
		}

		uint32_t index = swapchain->getAquiredImageIndex();

		commandPool->getCommandBuffer(index)->BindBuffer();
		renderpass->BeginRenderpass(commandPool->getCommandBuffer(index), primaryFramebuffers[index].get(), swapchain->getSwapchainExtent(), swapchain->getClearColour());
	}

	void VKRenderer::EndRendering()
	{
		VKSwapchain* swapchain = (VKSwapchain*)window->getSwapchain();
		uint32_t index = swapchain->getAquiredImageIndex();

		renderpass->EndRenderpass(commandPool->getCommandBuffer(index));
		commandPool->getCommandBuffer(index)->UnbindBuffer();

		window->Render(commandPool->getCommandBuffer(index));

		//Clear the lists for the next iteration
		models_3d.clear();
		models_transparency.clear();
		models_2d.clear();
		lights.clear();
	}

	void VKRenderer::Render()
	{
		CommandBuffer* buffer = (CommandBuffer*)commandPool->getCommandBuffer(window->getSwapchain()->getAquiredImageIndex());

		for (auto pipeline : models_3d)
		{
			Pipeline* pl = pipeline.first;

			pl->BindPipeline(buffer);
			pl->getShader()->UpdateShaderUniforms(camera, lights);
			pl->getShader()->UpdateSharedUniforms(pipeline.second[0]->getParent()); //Safe -> there is at least 1 element in the array

			for (size_t i = 0; i < pipeline.second.size(); i++)
			{
				pipeline.second[i]->BatchRender((uint32_t)i);
			}

			pl->UnbindPipeline();
		}

		//TODO: enable alpha blending
		for (auto pipeline : models_transparency)
		{
			Pipeline* pl = pipeline.first;

			pl->BindPipeline(buffer);
			pl->getShader()->UpdateShaderUniforms(camera, lights);
			pl->getShader()->UpdateSharedUniforms(pipeline.second[0]->getParent()); //Safe -> there is at least 1 element in the array

			for (size_t i = 0; i < pipeline.second.size(); i++)
			{
				pipeline.second[i]->BatchRender((uint32_t)i);
			}

			pl->UnbindPipeline();
		}

		//TODO: disable alpha blending and depth testing
		for (auto pipeline : models_2d)
		{
			Pipeline* pl = pipeline.first;

			pl->BindPipeline(buffer);

			for (size_t i = 0; i < pipeline.second.size(); i++)
			{
				pipeline.second[i]->BatchRender((uint32_t)i);
			}

			pl->UnbindPipeline();
		}
	}
};
