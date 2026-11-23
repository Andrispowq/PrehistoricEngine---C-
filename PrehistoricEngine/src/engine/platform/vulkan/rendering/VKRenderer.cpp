#include "Includes.hpp"
#include "VKRenderer.h"
#include "VKEnvironmentBackground.h"
#include "VKEnvironmentLighting.h"
#include "platform/vulkan/texture/VKTexture.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/core/resources/loaders/TextureLoader.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"

#include "platform/vulkan/framework/context/VKContext.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"

#include "prehistoric/core/resources/AssetManager.h"

namespace Prehistoric {
    VKRenderer::VKRenderer(Window *window, Camera *camera, AssembledAssetManager *manager)
        : Renderer(window, camera, manager), commandPool(nullptr), renderpass(nullptr) {
        VKContext *context = (VKContext *) window->getContext();
        VKSwapchain *swapchain = (VKSwapchain *) window->getSwapchain();

        VKDevice *device = (VKDevice *) context->getDevices();

        renderpass = std::make_unique<VKRenderpass>(device, swapchain);

        size_t NumImages = swapchain->getSwapchainImages().size();
        primaryFramebuffers.resize(NumImages);

        for (size_t i = 0; i < NumImages; i++) {
            primaryFramebuffers[i] = renderpass->CreateFramebuffer(static_cast<uint32_t>(i));
        }

        //Create command pool
        commandPool = std::make_unique<VKCommandPool>(device);

        for (size_t i = 0; i < NumImages; i++) {
            commandPool->AddCommandBuffer();
        }

        swapchain->setRenderpass(renderpass.get());
        //A nastly little cheat to avoid modifying everything in the Pipeline system because the Vulkan Graphics Pipelines require the renderpass
        background = std::make_unique<VKEnvironmentBackground>(window);

        VkDescriptorImageInfo source = background->getSource();
        std::unique_ptr<VKTexture> fallback;
        if (!source.imageView) {
            fallback.reset(
                (VKTexture *)
                TextureLoader::LoadTexture(window, "engine://textures/default.png", Bilinear, ClampToEdge));
            source = {
                fallback->getSampler(), fallback->getImage()->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };
        }

        // Cap startup work independently of the more expensive OpenGL bake settings.
        // Maps are immutable and survive swapchain recreation.
        uint32_t irradianceSize = std::clamp(uint32_t(__EnvironmentMapConfig.irradianceMapResolution), 1u, 64u);
        uint32_t prefilterSize = std::clamp(uint32_t(__EnvironmentMapConfig.prefilterMapResolution), 2u, 256u);
        uint32_t levels = std::clamp(uint32_t(__EnvironmentMapConfig.prefilterLevels), 2u, 9u);
        environmentLighting = std::make_unique<VKEnvironmentLighting>(device->getPhysicalDevice(), device->getDevice(),
                                                                      device->getGraphicsQueue().getQueue(),
                                                                      device->getQueueFamilyIndices().graphicsFamily,
                                                                      source,
                                                                      irradianceSize, prefilterSize, levels,
                                                                      __EnvironmentMapConfig.environmentMapResolution,
                                                                      ResourceLoader::LoadShaderVK("vulkan/ibl/convert_CS.spv"),
                                                                      ResourceLoader::LoadShaderVK(
                                                                          "vulkan/ibl/ibl_CS.spv"));
        if (background->getSource().imageView) background->SetPrefilter(environmentLighting->getPrefilter());
    }

    VKRenderer::~VKRenderer() {
        vkDeviceWaitIdle(((VKDevice *) window->getContext()->getDevices())->getDevice());
    }

    void VKRenderer::PrepareRendering() {
        VKSwapchain *swapchain = (VKSwapchain *) window->getSwapchain();
        frameReady = false;
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize((GLFWwindow *) window->getWindowHandle(), &framebufferWidth, &framebufferHeight);
        if (framebufferWidth == 0 || framebufferHeight == 0)
            return;

        if (window->isResized()) {
            PR_PROFILE("Resizing window");

            VKContext *context = (VKContext *) window->getContext();
            VKDevice *device = (VKDevice *) context->getDevices();
            vkDeviceWaitIdle(device->getDevice());

            primaryFramebuffers.clear();
            delete renderpass.release();

            swapchain->SetWindowSize((uint32_t) framebufferWidth, (uint32_t) framebufferHeight);
            VkExtent2D extent = swapchain->getSwapchainExtent();
            uint32_t width = extent.width;
            uint32_t height = extent.height;
            camera->SetProjection(camera->getFov(), (float) width, (float) height);

            renderpass = std::make_unique<VKRenderpass>(device, swapchain);
            swapchain->setRenderpass(renderpass.get());

            size_t NumImages = swapchain->getSwapchainImages().size();
            primaryFramebuffers.resize(NumImages);
            for (size_t i = 0; i < NumImages; i++) {
                primaryFramebuffers[i] = renderpass->CreateFramebuffer(static_cast<uint32_t>(i));
            }

            //Create command pool
            commandPool->DeleteCommandBuffers();
            for (size_t i = 0; i < NumImages; i++) {
                commandPool->AddCommandBuffer();
            }

            //Recreate the pipelines
            std::vector<Pipeline *> pipes = manager->get<Pipeline>();
            for (Pipeline *pipe: pipes) {
                pipe->setViewportSize({(float) width, (float) height});
                pipe->setScissorSize({width, height});
                pipe->RecreatePipeline();
            }

            background->RecreatePipeline();

            window->setResized(false);
            commandBuffersReady = false;
        }

        if (!swapchain->GetNextImageIndex())
            return;
        frameReady = true;

        if (!commandBuffersReady) {
            BuildCommandBuffers();
            commandBuffersReady = true;
        }

        /*uint32_t index = swapchain->getAquiredImageIndex();
		VKCommandBuffer* buffer = commandPool->getCommandBuffer(index);

		buffer->BindBuffer();
		renderpass->BeginRenderpass(buffer, primaryFramebuffers[index].get(), swapchain->getSwapchainExtent(), swapchain->getClearColour());*/
    }

    void VKRenderer::EndRendering() {
        if (frameReady) {
            VKSwapchain *swapchain = (VKSwapchain *) window->getSwapchain();
            uint32_t index = swapchain->getAquiredImageIndex();
            VKCommandBuffer *buffer = commandPool->getCommandBuffer(index);

            /*renderpass->EndRenderpass(buffer);
			buffer->UnbindBuffer();*/

            {
                PR_PROFILE("Submitting");
                window->Render(buffer);
            }
        }

        //Clear the lists
        models_3d.clear();
        models_transparency.clear();
        models_2d.clear();
        lights.clear();
    }

    void VKRenderer::Render() {
        if (!frameReady)
            return;

        PR_PROFILE("Update uniforms");

        VKSwapchain *swapchain = (VKSwapchain *) window->getSwapchain();
        uint32_t index = swapchain->getAquiredImageIndex();
        VKCommandBuffer *buffer = commandPool->getCommandBuffer(index);

        uint32_t matIdx = 0;
        uint32_t instIdx = 0;
        for (auto pipeline: models_3d) {
            Pipeline *pl = pipeline.first;
            pl->getShader()->UpdateGlobalUniforms(this);

            for (auto material: pipeline.second) {
                pl->getShader()->UpdateMaterialUniforms(material.first, (uint32_t) matIdx++);

                //instIdx = 0;
                for (size_t i = 0; i < material.second.size(); i++) {
                    pl->getShader()->UpdateObjectUniforms(material.second[i]->getParent(), (uint32_t) instIdx++);
                }
            }
        }

        //TODO: enable alpha blending
        for (auto pipeline: models_transparency) {
            Pipeline *pl = pipeline.first;
            pl->getShader()->UpdateGlobalUniforms(this);

            for (auto material: pipeline.second) {
                pl->getShader()->UpdateMaterialUniforms(material.first, (uint32_t) matIdx++);

                for (size_t i = 0; i < material.second.size(); i++) {
                    pl->getShader()->UpdateObjectUniforms(material.second[i]->getParent(), (uint32_t) instIdx++);
                }
            }
        }

        //TODO: disable alpha blending and depth testing
        for (auto pipeline: models_2d) {
            Pipeline *pl = pipeline.first;

            instIdx = 0;
            for (size_t i = 0; i < pipeline.second.size(); i++) {
                pl->getShader()->UpdateObjectUniforms(pipeline.second[i]->getParent(), instIdx++);
            }
        }

        BuildCommandBuffers();
    }

    void VKRenderer::BuildCommandBuffers() {
        PR_PROFILE("Recording command buffers");

        VKSwapchain *swapchain = (VKSwapchain *) window->getSwapchain();
        for (uint32_t i = 0; i < swapchain->getSwapchainImages().size(); i++) {
            VKCommandBuffer *buffer = commandPool->getCommandBuffer(i);

            buffer->BindBuffer();
            renderpass->BeginRenderpass(buffer, primaryFramebuffers[i].get(), swapchain->getSwapchainExtent(),
                                        swapchain->getClearColour());
            background->Draw(buffer->getCommandBuffer(), camera);

            uint32_t matIdx = 0;
            uint32_t instIdx = 0;
            for (auto pipeline: models_3d) {
                Pipeline *pl = pipeline.first;
                pl->BindPipeline(buffer);
                ((VKShader *) pl->getShader())->BindGlobalSets();

                for (auto material: pipeline.second) {
                    ((VKShader *) pl->getShader())->BindTextureSets(matIdx++);

                    //instIdx = 0;
                    for (size_t i = 0; i < material.second.size(); i++) {
                        ((VKShader *) pl->getShader())->BindObjectSets(instIdx++);
                        pl->RenderPipeline();
                    }
                }

                pl->UnbindPipeline();
            }

            //TODO: enable alpha blending
            for (auto pipeline: models_transparency) {
                Pipeline *pl = pipeline.first;
                pl->BindPipeline(buffer);
                ((VKShader *) pl->getShader())->BindGlobalSets();

                for (auto material: pipeline.second) {
                    ((VKShader *) pl->getShader())->BindTextureSets(matIdx++);

                    for (size_t i = 0; i < material.second.size(); i++) {
                        ((VKShader *) pl->getShader())->BindObjectSets(instIdx++);
                        pl->RenderPipeline();
                    }
                }

                pl->UnbindPipeline();
            }

            //TODO: disable alpha blending and depth testing
            for (auto pipeline: models_2d) {
                Pipeline *pl = pipeline.first;
                pl->BindPipeline(buffer);

                for (size_t i = 0; i < pipeline.second.size(); i++) {
                    ((VKShader *) pl->getShader())->BindObjectSets(instIdx++);
                    pl->RenderPipeline();
                }

                pl->UnbindPipeline();
            }

            renderpass->EndRenderpass(buffer);
            buffer->UnbindBuffer();
        }
    }
};