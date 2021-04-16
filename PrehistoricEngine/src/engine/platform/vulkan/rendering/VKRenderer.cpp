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

		VKDevice* device = (VKDevice*)context->getDevices();

		renderpass = std::make_unique<VKRenderpass>(device, swapchain);

		size_t NumImages = swapchain->getSwapchainImages().size();
		primaryFramebuffers.resize(NumImages);

		for (size_t i = 0; i < NumImages; i++)
		{
			VkImageView attachments[] =
			{
				swapchain->getColourImage()->getImageView(),
				swapchain->getDepthImage()->getImageView(),
				swapchain->getSwapchainImages()[i]->getImageView()
			};

			primaryFramebuffers[i] = std::make_unique<VKFramebuffer>(device->getDevice(), renderpass->getRenderPass(), swapchain->getSwapchainExtent().width,
				swapchain->getSwapchainExtent().height, 3, attachments);
		}

		//Create command pool
		commandPool = std::make_unique<VKCommandPool>(device);

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

			VKContext* context = (VKContext*)window->getContext();
			VKSwapchain* swapchain = (VKSwapchain*)window->getSwapchain();

			camera->SetProjection(camera->getFov(), (float)width, (float)height);
			swapchain->SetWindowSize(width, height);

			VKDevice* device = (VKDevice*)context->getDevices();

			renderpass = std::make_unique<VKRenderpass>(device, swapchain);
			swapchain->setRenderpass(renderpass.get());

			size_t NumImages = swapchain->getSwapchainImages().size();
			primaryFramebuffers.resize(NumImages);
			for (size_t i = 0; i < NumImages; i++)
			{
				VkImageView attachments[] =
				{
					swapchain->getColourImage()->getImageView(),
					swapchain->getDepthImage()->getImageView(),
					swapchain->getSwapchainImages()[i]->getImageView()
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
		VKCommandBuffer* buffer = commandPool->getCommandBuffer(index);

		buffer->BindBuffer();
		renderpass->BeginRenderpass(buffer, primaryFramebuffers[index].get(), swapchain->getSwapchainExtent(), swapchain->getClearColour());
	}

	void VKRenderer::EndRendering()
	{
		VKSwapchain* swapchain = (VKSwapchain*)window->getSwapchain();
		uint32_t index = swapchain->getAquiredImageIndex();
		VKCommandBuffer* buffer = commandPool->getCommandBuffer(index);

		renderpass->EndRenderpass(buffer);
		buffer->UnbindBuffer();

		window->Render(commandPool->getCommandBuffer(index));

		//Clear the lists
		models_3d.clear();
		models_transparency.clear();
		models_2d.clear();
		lights.clear();
	}

	void VKRenderer::Render()
	{
		VKSwapchain* swapchain = (VKSwapchain*)window->getSwapchain();
		uint32_t index = swapchain->getAquiredImageIndex();
		VKCommandBuffer* buffer = commandPool->getCommandBuffer(index);

		for (auto pipeline : models_3d)
		{
			Pipeline* pl = pipeline.first;

			pl->BindPipeline(buffer);
			pl->getShader()->UpdateGlobalUniforms(camera, lights);

			uint32_t counter = 0;
			for (auto material : pipeline.second)
			{
				pl->getShader()->UpdateTextureUniforms(material.first, counter++);

				for (size_t i = 0; i < material.second.size(); i++)
				{
					material.second[i]->BatchRender((uint32_t)i);
				}
			}

			pl->UnbindPipeline();
		}

		//TODO: enable alpha blending
		for (auto pipeline : models_transparency)
		{
			Pipeline* pl = pipeline.first;

			pl->BindPipeline(buffer);
			pl->getShader()->UpdateGlobalUniforms(camera, lights);

			uint32_t counter = 0;
			for (auto material : pipeline.second)
			{
				pl->getShader()->UpdateTextureUniforms(material.first, counter++);

				for (size_t i = 0; i < material.second.size(); i++)
				{
					material.second[i]->BatchRender((uint32_t)i);
				}
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

	void VKRenderer::BuildCommandBuffers()
	{
	}
};
