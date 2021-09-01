#include "Includes.hpp"
#include "VKRenderer.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"

#include "platform/vulkan/framework/context/VKContext.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"

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
			PR_PROFILE("Resizing window");

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
			commandBuffersReady = false;
		}

		if (!commandBuffersReady)
		{
			BuildCommandBuffers();
			commandBuffersReady = true;
		}

		/*uint32_t index = swapchain->getAquiredImageIndex();
		VKCommandBuffer* buffer = commandPool->getCommandBuffer(index);

		buffer->BindBuffer();
		renderpass->BeginRenderpass(buffer, primaryFramebuffers[index].get(), swapchain->getSwapchainExtent(), swapchain->getClearColour());*/
	}

	void VKRenderer::EndRendering()
	{
		VKSwapchain* swapchain = (VKSwapchain*)window->getSwapchain();
		uint32_t index = swapchain->getAquiredImageIndex();
		VKCommandBuffer* buffer = commandPool->getCommandBuffer(index);

		/*renderpass->EndRenderpass(buffer);
		buffer->UnbindBuffer();*/

		{
			PR_PROFILE("Submitting");
			window->Render(buffer);
		}

		//Clear the lists
		models_3d.clear();
		models_transparency.clear();
		models_2d.clear();
		lights.clear();
	}

	void VKRenderer::Render()
	{
		PR_PROFILE("Update uniforms");

		VKSwapchain* swapchain = (VKSwapchain*)window->getSwapchain();
		uint32_t index = swapchain->getAquiredImageIndex();
		VKCommandBuffer* buffer = commandPool->getCommandBuffer(index);

		uint32_t matIdx = 0;
		uint32_t instIdx = 0;
		for (auto pipeline : models_3d)
		{
			Pipeline* pl = pipeline.first;
			pl->getShader()->UpdateGlobalUniforms(camera, lights);

			for (auto material : pipeline.second)
			{
				pl->getShader()->UpdateMaterialUniforms(material.first, (uint32_t)matIdx++);

				for (size_t i = 0; i < material.second.size(); i++)
				{
					pl->getShader()->UpdateObjectUniforms(material.second[i]->getParent(), (uint32_t)instIdx++);
				}
			}
		}

		//TODO: enable alpha blending
		for (auto pipeline : models_transparency)
		{
			Pipeline* pl = pipeline.first;
			pl->getShader()->UpdateGlobalUniforms(camera, lights);

			for (auto material : pipeline.second)
			{
				pl->getShader()->UpdateMaterialUniforms(material.first, (uint32_t)matIdx++);

				for (size_t i = 0; i < material.second.size(); i++)
				{
					pl->getShader()->UpdateObjectUniforms(material.second[i]->getParent(), (uint32_t)instIdx++);
				}
			}
		}

		//TODO: disable alpha blending and depth testing
		for (auto pipeline : models_2d)
		{
			Pipeline* pl = pipeline.first;

			instIdx = 0;
			for (size_t i = 0; i < pipeline.second.size(); i++)
			{
				pl->getShader()->UpdateObjectUniforms(pipeline.second[i]->getParent(), instIdx++);
			}
		}

		BuildCommandBuffers();
	}

	void VKRenderer::BuildCommandBuffers()
	{
		PR_PROFILE("Recording command buffers");

		VKSwapchain* swapchain = (VKSwapchain*)window->getSwapchain();
		for (uint32_t i = 0; i < swapchain->getSwapchainImages().size(); i++)
		{
			VKCommandBuffer* buffer = commandPool->getCommandBuffer(i);

			buffer->BindBuffer();
			renderpass->BeginRenderpass(buffer, primaryFramebuffers[i].get(), swapchain->getSwapchainExtent(), swapchain->getClearColour());

			uint32_t matIdx = 0;
			uint32_t instIdx = 0;
			for (auto pipeline : models_3d)
			{
				Pipeline* pl = pipeline.first;
				pl->BindPipeline(buffer);
				((VKShader*)pl->getShader())->BindGlobalSets();

				for (auto material : pipeline.second)
				{
					((VKShader*)pl->getShader())->BindTextureSets(matIdx++);

					for (size_t i = 0; i < material.second.size(); i++)
					{
						((VKShader*)pl->getShader())->BindObjectSets(instIdx++);
						pl->RenderPipeline();
					}
				}

				pl->UnbindPipeline();
			}

			//TODO: enable alpha blending
			for (auto pipeline : models_transparency)
			{
				Pipeline* pl = pipeline.first;
				pl->BindPipeline(buffer);
				((VKShader*)pl->getShader())->BindGlobalSets();

				for (auto material : pipeline.second)
				{
					((VKShader*)pl->getShader())->BindTextureSets(matIdx++);

					for (size_t i = 0; i < material.second.size(); i++)
					{
						((VKShader*)pl->getShader())->BindObjectSets(instIdx++);
						pl->RenderPipeline();
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
					((VKShader*)pl->getShader())->BindObjectSets(instIdx++);
					pl->RenderPipeline();
				}

				pl->UnbindPipeline();
			}

			renderpass->EndRenderpass(buffer);
			buffer->UnbindBuffer();
		}
	}
};
