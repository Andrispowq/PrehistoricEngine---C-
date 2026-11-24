#include "Includes.hpp"
#include "VKRenderpass.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/common/rendering/shaders/Shader.h"
#include "platform/vulkan/rendering/shaders/VKColourSpace.h"

#include "platform/vulkan/framework/swapchain/VKSwapchain.h"

namespace Prehistoric {
    static void Check(VkResult result) {
        if (result != VK_SUCCESS) {
            PR_LOG_RUNTIME_ERROR("Vulkan fullscreen pass creation failed (%d)!\n", result);
        }
    }

    VkPipeline VKRenderpass::CreateFullscreenPipeline(const char *vertexPath, const char *fragmentPath,
                                                      VkPipelineLayout layout, VkRenderPass postPass) {
        VkShaderModule modules[2]{};
        VkPipelineShaderStageCreateInfo shaderStages[2]{};
        const char *paths[] = {vertexPath, fragmentPath};

        for (size_t i = 0; i < 2; ++i) {
            auto code = ResourceLoader::LoadShaderVK(paths[i]);
            VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            moduleInfo.codeSize = code.size();
            moduleInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
            Check(vkCreateShaderModule(device->getDevice(), &moduleInfo, nullptr, &modules[i]));

            shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[i].stage = i == 0 ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[i].module = modules[i];
            shaderStages[i].pName = "main";
        }

        VkPipelineVertexInputStateCreateInfo vertex{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo viewport{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewport.viewportCount = viewport.scissorCount = 1;

        VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamic.dynamicStateCount = 2;
        dynamic.pDynamicStates = dynamicStates;

        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.cullMode = VK_CULL_MODE_NONE;
        raster.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisample.rasterizationSamples = !postPass ? swapchain->getSceneSamples() : VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depth{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depth.depthTestEnable = !postPass;
        depth.depthWriteEnable = VK_FALSE;
        depth.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        VkPipelineColorBlendAttachmentState attachment{};
        attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                    VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        VkPipelineColorBlendAttachmentState attachments[] = {attachment, attachment};
        blend.attachmentCount = postPass ? 1 : 2;
        blend.pAttachments = attachments;

        VkGraphicsPipelineCreateInfo info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        info.stageCount = 2;
        info.pStages = shaderStages;
        info.pVertexInputState = &vertex;
        info.pInputAssemblyState = &assembly;
        info.pViewportState = &viewport;
        info.pDynamicState = &dynamic;
        info.pRasterizationState = &raster;
        info.pMultisampleState = &multisample;
        info.pDepthStencilState = &depth;
        info.pColorBlendState = &blend;
        info.layout = layout;
        info.renderPass = postPass ? postPass : renderpass;
        VkPipeline pipeline{};
        const VkResult result = vkCreateGraphicsPipelines(device->getDevice(), VK_NULL_HANDLE, 1, &info, nullptr,
                                                          &pipeline);

        for (auto module: modules) {
            vkDestroyShaderModule(device->getDevice(), module, nullptr);
        }
        Check(result);
        return pipeline;
    }

    VKRenderpass::VKRenderpass(VKDevice *device, VKSwapchain *swapchain)
        : device(device), swapchain(swapchain) {
        CreateScenePass();
        CreatePostProcessing();
    }

    VKImage *VKRenderpass::CreateImage(VkExtent2D extent, VkSampleCountFlagBits samples) {
        VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        usage |= samples == VK_SAMPLE_COUNT_1_BIT
                     ? VK_IMAGE_USAGE_SAMPLED_BIT
                     : VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        images.push_back(std::make_unique<VKImage>(device, extent.width, extent.height, 1,
                                                   swapchain->getHDRImage()->getImageFormat(), VK_IMAGE_TILING_OPTIMAL,
                                                   usage, samples,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT));
        return images.back().get();
    }

    VKFramebuffer *VKRenderpass::CreateFramebuffer(VkRenderPass pass, VkExtent2D extent, uint32_t count,
                                                   VkImageView *views) {
        framebuffers.push_back(
            std::make_unique<VKFramebuffer>(device->getDevice(), pass, extent.width, extent.height, count, views));
        return framebuffers.back().get();
    }

    static VkAttachmentDescription Attachment(VkFormat format, VkSampleCountFlagBits samples,
                                              VkAttachmentLoadOp load, VkAttachmentStoreOp store,
                                              VkImageLayout finalLayout) {
        VkAttachmentDescription attachment{};
        attachment.format = format;
        attachment.samples = samples;
        attachment.loadOp = load;
        attachment.storeOp = store;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.finalLayout = finalLayout;
        return attachment;
    }

    static VkRenderPass CreatePass(VkDevice device, const VkSubpassDescription &subpass,
                                   uint32_t count, const VkAttachmentDescription *attachments) {
        // Targets are reused across frames. Order earlier sampling and attachment writes
        // before reuse, and make each colour result visible to subsequent fullscreen draws.
        const VkPipelineStageFlags writes = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        VkSubpassDependency dependencies[] =
        {
            {
                VK_SUBPASS_EXTERNAL, 0, writes | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, writes,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                0
            },
            {
                0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 0
            }
        };
        VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        info.attachmentCount = count;
        info.pAttachments = attachments;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 2;
        info.pDependencies = dependencies;
        VkRenderPass pass{};
        Check(vkCreateRenderPass(device, &info, nullptr, &pass));
        return pass;
    }

    void VKRenderpass::CreateScenePass() {
        VkExtent2D extent = swapchain->getSwapchainExtent();
        auto samples = swapchain->getSceneSamples();
        bool resolve = samples != VK_SAMPLE_COUNT_1_BIT;
        emission = CreateImage(extent, VK_SAMPLE_COUNT_1_BIT);
        emissionTarget = resolve ? CreateImage(extent, samples) : emission;
        auto colour = Attachment(emission->getImageFormat(), samples, VK_ATTACHMENT_LOAD_OP_CLEAR,
                                 resolve ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE,
                                 resolve
                                     ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                     : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        auto depth = Attachment(swapchain->getDepthImage()->getImageFormat(), samples, VK_ATTACHMENT_LOAD_OP_CLEAR,
                                VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        auto resolved = Attachment(colour.format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                   VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        VkAttachmentDescription attachments[] = {colour, colour, depth, resolved, resolved};
        VkImageView views[] = {
            swapchain->getColourImage()->getImageView(), emissionTarget->getImageView(),
            swapchain->getDepthImage()->getImageView(), swapchain->getHDRImage()->getImageView(),
            emission->getImageView()
        };
        VkAttachmentReference colours[] = {
            {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
        };
        VkAttachmentReference resolves[] = {
            {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, {4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
        };
        VkAttachmentReference depthRef{2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        VkSubpassDescription scene{};
        scene.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        scene.colorAttachmentCount = 2;
        scene.pColorAttachments = colours;
        scene.pDepthStencilAttachment = &depthRef;
        if (resolve) scene.pResolveAttachments = resolves;
        uint32_t count = resolve ? 5 : 3;
        renderpass = CreatePass(device->getDevice(), scene, count, attachments);
        sceneFramebuffer = CreateFramebuffer(renderpass, extent, count, views);
    }

    VKRenderpass::WaterTargets& VKRenderpass::GetWaterTargets(const Water* water)
    {
        auto& targets = waterTargets[water];
        if (targets.reflectionFrame) return targets;
        const auto extent = swapchain->getSwapchainExtent();
        bool resolve = swapchain->getSceneSamples() != VK_SAMPLE_COUNT_1_BIT;
        auto make = [&](std::unique_ptr<VKTexture>& texture, std::unique_ptr<VKFramebuffer>& framebuffer)
        {
            texture = VKTexture::CreateColourTarget(device, extent, emission->getImageFormat());
            VkImageView colour = texture->getImage()->getImageView();
            VkImageView views[] = {resolve ? swapchain->getColourImage()->getImageView() : colour,
                emissionTarget->getImageView(), swapchain->getDepthImage()->getImageView(), colour, emission->getImageView()};
            framebuffer = std::make_unique<VKFramebuffer>(device->getDevice(), renderpass, extent.width, extent.height, resolve ? 5 : 3, views);
        };
        make(targets.reflection,targets.reflectionFrame);
        make(targets.refraction,targets.refractionFrame);
        return targets;
    }

    void VKRenderpass::RetainWaterTargets(const std::vector<Water*>& active)
    {
        for (auto it = waterTargets.begin(); it != waterTargets.end(); )
        {
            if (std::find(active.begin(), active.end(), it->first) == active.end()) it = waterTargets.erase(it);
            else ++it;
        }
    }

    VkRenderPass VKRenderpass::CreateOutputPass(VkFormat format, VkImageLayout finalLayout) {
        auto output = Attachment(format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                 VK_ATTACHMENT_STORE_OP_STORE, finalLayout);
        VkAttachmentReference colour{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colour;
        return CreatePass(device->getDevice(), subpass, 1, &output);
    }

    VkDescriptorSet VKRenderpass::CreateInputs(VkImageView source, VkImageView second) {
        VkDevice handle = device->getDevice();
        VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        alloc.descriptorPool = hdrPool;
        alloc.descriptorSetCount = 1;
        alloc.pSetLayouts = &hdrSetLayout;
        VkDescriptorSet set{};
        Check(vkAllocateDescriptorSets(handle, &alloc, &set));
        VkDescriptorImageInfo inputs[] = {
            {sampler, source, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
            {sampler, second, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}
        };
        VkWriteDescriptorSet writes[2]{};
        for (uint32_t i = 0; i < 2; ++i) {
            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = set;
            writes[i].dstBinding = i;
            writes[i].descriptorCount = 1;
            writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[i].pImageInfo = &inputs[i];
        }
        vkUpdateDescriptorSets(handle, 2, writes, 0, nullptr);
        return set;
    }

    void VKRenderpass::CreatePostProcessing() {
        constexpr uint32_t levels = 8;
        constexpr uint32_t maxSets = levels * 2 + (levels - 1) + 2;
        VkDevice handle = device->getDevice();
        VkExtent2D extent = swapchain->getSwapchainExtent();
        bloomPass = CreateOutputPass(emission->getImageFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        presentPass = CreateOutputPass(swapchain->getSwapchainImageFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        for (auto *image: swapchain->getSwapchainImages()) {
            auto view = image->getImageView();
            presentFramebuffers.push_back(CreateFramebuffer(presentPass, extent, 1, &view)->getFramebuffer());
        }

        VkSamplerCreateInfo sampling{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        sampling.magFilter = sampling.minFilter = VK_FILTER_LINEAR;
        sampling.addressModeU = sampling.addressModeV = sampling.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        Check(vkCreateSampler(handle, &sampling, nullptr, &sampler));
        VkDescriptorSetLayoutBinding bindings[] =
        {
            {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
        };
        VkDescriptorSetLayoutCreateInfo setInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        setInfo.bindingCount = 2;
        setInfo.pBindings = bindings;
        Check(vkCreateDescriptorSetLayout(handle, &setInfo, nullptr, &hdrSetLayout));
        VkDescriptorPoolSize size{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxSets * 2};
        VkDescriptorPoolCreateInfo pool{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        pool.maxSets = maxSets;
        pool.poolSizeCount = 1;
        pool.pPoolSizes = &size;
        Check(vkCreateDescriptorPool(handle, &pool, nullptr, &hdrPool));
        VkPushConstantRange range{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4 * sizeof(float)};
        VkPipelineLayoutCreateInfo layout{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        layout.setLayoutCount = layout.pushConstantRangeCount = 1;
        layout.pSetLayouts = &hdrSetLayout;
        layout.pPushConstantRanges = &range;
        Check(vkCreatePipelineLayout(handle, &layout, nullptr, &hdrLayout));
        const char *vertex = "vulkan/postProcessing/hdr_VS.spv";
        bloomPipeline = CreateFullscreenPipeline(vertex, "vulkan/postProcessing/bloom_FS.spv", hdrLayout, bloomPass);
        if (Application::Get().isEditor()) {
            editorOutput = VKTexture::CreateColourTarget(device, extent);
            auto *image = editorOutput->getImage();
            auto view = image->getImageView();
            editorPass = CreateOutputPass(image->getImageFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            editorFramebuffer = CreateFramebuffer(editorPass, extent, 1, &view)->getFramebuffer();
        }
        hdrPipeline = CreateFullscreenPipeline(vertex, "vulkan/postProcessing/hdr_FS.spv", hdrLayout,
                                               editorOutput ? editorPass : presentPass);

        struct Level {
            VkExtent2D extent;
            VkImageView vertical;
            VkImageView blurred;
            VkFramebuffer verticalFramebuffer;
        };
        std::vector<Level> pyramid;
        VkImageView source = emission->getImageView();
        for (uint32_t i = 0; i < levels; ++i) {
            extent = {std::max(1u, extent.width / 2), std::max(1u, extent.height / 2)};
            auto vertical = CreateImage(extent, VK_SAMPLE_COUNT_1_BIT)->getImageView();
            auto blurred = CreateImage(extent, VK_SAMPLE_COUNT_1_BIT)->getImageView();
            auto first = CreateFramebuffer(bloomPass, extent, 1, &vertical)->getFramebuffer();
            auto second = CreateFramebuffer(bloomPass, extent, 1, &blurred)->getFramebuffer();
            bloomDraws.push_back({first, extent, CreateInputs(source, source), {0, 1.0f / extent.height, 0}});
            bloomDraws.push_back({second, extent, CreateInputs(vertical, vertical), {1.0f / extent.width, 0, 0}});
            pyramid.push_back({extent, vertical, blurred, first});
            source = blurred;
            if (extent.width == 1 && extent.height == 1) break;
        }
        for (size_t i = pyramid.size() - 1; i > 0; --i) {
            const auto &level = pyramid[i - 1];
            bloomDraws.push_back({
                level.verticalFramebuffer, level.extent, CreateInputs(level.blurred, source), {0, 0, 1}
            });
            source = level.vertical;
        }
        auto scene = swapchain->getHDRImage()->getImageView();
        hdrInputs[0] = CreateInputs(scene, scene);
        hdrInputs[1] = CreateInputs(scene, source);
    }

    VKRenderpass::~VKRenderpass() {
        VkDevice handle = device->getDevice();
        waterTargets.clear();
        framebuffers.clear();
        vkDestroyRenderPass(handle, editorPass, nullptr);
        vkDestroyPipeline(handle, hdrPipeline, nullptr);
        vkDestroyPipeline(handle, bloomPipeline, nullptr);
        vkDestroyPipelineLayout(handle, hdrLayout, nullptr);
        vkDestroyDescriptorPool(handle, hdrPool, nullptr);
        vkDestroyDescriptorSetLayout(handle, hdrSetLayout, nullptr);
        vkDestroySampler(handle, sampler, nullptr);
        vkDestroyRenderPass(handle, presentPass, nullptr);
        vkDestroyRenderPass(handle, bloomPass, nullptr);
        vkDestroyRenderPass(handle, renderpass, nullptr);
    }

    void VKRenderpass::BeginRenderpass(VKCommandBuffer *commandBuffer, Vector4f clearColour, VkFramebuffer target) {
        VkClearValue clear[3]{};
        clear[0].color = {clearColour.x, clearColour.y, clearColour.z, clearColour.w};
        clear[2].depthStencil = {1, 0};
        VkRenderPassBeginInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        info.renderPass = renderpass;
        info.framebuffer = target ? target : sceneFramebuffer->getFramebuffer();
        info.renderArea.extent = swapchain->getSwapchainExtent();
        info.clearValueCount = 3;
        info.pClearValues = clear;
        vkCmdBeginRenderPass(commandBuffer->getCommandBuffer(), &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VKRenderpass::RenderHDR(VKCommandBuffer *commandBuffer, bool bloomEnabled, bool underwater) {
        VkCommandBuffer command = commandBuffer->getCommandBuffer();
        vkCmdEndRenderPass(command);
        auto draw = [&](VkRenderPass pass, VkPipeline pipeline, const BloomDraw &draw) {
            VkRenderPassBeginInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            info.renderPass = pass;
            info.framebuffer = draw.framebuffer;
            info.renderArea.extent = draw.extent;
            vkCmdBeginRenderPass(command, &info, VK_SUBPASS_CONTENTS_INLINE);
            VkViewport viewport{0, 0, float(draw.extent.width), float(draw.extent.height), 0, 1};
            vkCmdSetViewport(command, 0, 1, &viewport);
            vkCmdSetScissor(command, 0, 1, &info.renderArea);
            vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, hdrLayout, 0, 1, &draw.inputs, 0,
                                    nullptr);
            vkCmdPushConstants(command, hdrLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(draw.settings),
                               draw.settings);
            vkCmdDraw(command, 3, 1, 0, 0);
        };
        if (bloomEnabled) {
            for (const auto &stage: bloomDraws) {
                draw(bloomPass, bloomPipeline, stage);
                vkCmdEndRenderPass(command);
            }
        }
        auto extent = swapchain->getSwapchainExtent();
        auto framebuffer = presentFramebuffers[swapchain->getAquiredImageIndex()];
        draw(editorOutput ? editorPass : presentPass, hdrPipeline,
             {
                 editorOutput ? editorFramebuffer : framebuffer, extent,
                 hdrInputs[bloomEnabled], {
                     __EngineConfig.rendererExposure,
                     VKPBRShaderGamma(swapchain->getSwapchainImageFormat(), __EngineConfig.rendererGamma),
                     float(bloomEnabled), float(underwater)
                 }
             });
        if (editorOutput) {
            vkCmdEndRenderPass(command);
            VkRenderPassBeginInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            info.renderPass = presentPass;
            info.framebuffer = framebuffer;
            info.renderArea.extent = extent;
            vkCmdBeginRenderPass(command, &info, VK_SUBPASS_CONTENTS_INLINE);
            // Uncovered areas can occur while docking or moving panels to another window.
            VkClearAttachment clear{};
            clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            clear.clearValue.color.float32[3] = 1;
            VkClearRect rect{info.renderArea, 0, 1};
            vkCmdClearAttachments(command, 1, &clear, 1, &rect);
        }
    }
}