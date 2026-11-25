#include "Includes.hpp"
#include "VKShadowPass.h"

#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"
#include "prehistoric/application/Application.h"

namespace Prehistoric {
    static void CheckShadow(VkResult result) {
        if (result != VK_SUCCESS)
            throw std::runtime_error("Vulkan shadow pass creation failed (" + std::to_string(result) + ")");
    }

    void VKShadowPass::CreatePipeline() {
        VkDevice device = gpu->getDevice();
        VkAttachmentDescription depth{};
        depth.format = VK_FORMAT_D32_SFLOAT;
        depth.samples = VK_SAMPLE_COUNT_1_BIT;
        depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        VkAttachmentReference attachment{0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pDepthStencilAttachment = &attachment;
        constexpr VkPipelineStageFlags tests = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                               VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        VkSubpassDependency dependencies[] = {
            {
                VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | tests, tests,
                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, 0
            },
            {
                0, VK_SUBPASS_EXTERNAL, tests, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 0
            }
        };
        VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        info.attachmentCount = info.subpassCount = 1;
        info.pAttachments = &depth;
        info.pSubpasses = &subpass;
        info.dependencyCount = 2;
        info.pDependencies = dependencies;
        CheckShadow(vkCreateRenderPass(device, &info, nullptr, &pass));

        VkSamplerCreateInfo sampling{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        sampling.addressModeU = sampling.addressModeV = sampling.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        CheckShadow(vkCreateSampler(device, &sampling, nullptr, &sampler));
        VkPushConstantRange range{VK_SHADER_STAGE_VERTEX_BIT, 0, 64};
        VkPipelineLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &range;
        CheckShadow(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout));

        const auto code = ResourceLoader::LoadShaderVK("vulkan/shadow/depth.vert.spv");
        VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        moduleInfo.codeSize = code.size();
        moduleInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
        VkShaderModule module{};
        CheckShadow(vkCreateShaderModule(device, &moduleInfo, nullptr, &module));
        VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage.module = module;
        stage.pName = "main";
        VkVertexInputBindingDescription binding{0, uint32_t(Vertex::getSize()), VK_VERTEX_INPUT_RATE_VERTEX};
        VkVertexInputAttributeDescription position{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
        VkPipelineVertexInputStateCreateInfo input{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        input.vertexBindingDescriptionCount = input.vertexAttributeDescriptionCount = 1;
        input.pVertexBindingDescriptions = &binding;
        input.pVertexAttributeDescriptions = &position;
        VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPipelineViewportStateCreateInfo viewport{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewport.viewportCount = viewport.scissorCount = 1;
        VkDynamicState states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamic.dynamicStateCount = 2;
        dynamic.pDynamicStates = states;
        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.lineWidth = 1.0f;
        VkPipelineMultisampleStateCreateInfo samples{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        samples.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPipelineDepthStencilStateCreateInfo test{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        test.depthTestEnable = test.depthWriteEnable = VK_TRUE;
        test.depthCompareOp = VK_COMPARE_OP_LESS;
        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        VkGraphicsPipelineCreateInfo create{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        create.stageCount = 1;
        create.pStages = &stage;
        create.pVertexInputState = &input;
        create.pInputAssemblyState = &assembly;
        create.pViewportState = &viewport;
        create.pRasterizationState = &raster;
        create.pMultisampleState = &samples;
        create.pDepthStencilState = &test;
        create.pColorBlendState = &blend;
        create.pDynamicState = &dynamic;
        create.layout = layout;
        create.renderPass = pass;
        VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create, nullptr, &pipeline);
        vkDestroyShaderModule(device, module, nullptr);
        CheckShadow(result);
    }

    void VKShadowPass::ReleaseTargets() {
        for (auto &frame: frames) frame.reset();
        for (auto &view: views) {
            vkDestroyImageView(gpu->getDevice(), view, nullptr);
            view = VK_NULL_HANDLE;
        }
        image.reset();
    }

    void VKShadowPass::CreateTargets(unsigned size) {
        if (image) {
            CheckShadow(vkDeviceWaitIdle(gpu->getDevice()));
            ReleaseTargets();
        }
        resolution = size;
        initialized = false;
        image = std::make_unique<VKImage>(gpu, size, size, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VK_SAMPLE_COUNT_1_BIT,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
                                          ShadowCascades::Count);
        for (unsigned i = 0; i < ShadowCascades::Count; ++i) {
            VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            view.image = image->getImage();
            view.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view.format = image->getImageFormat();
            view.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, i, 1};
            CheckShadow(vkCreateImageView(gpu->getDevice(), &view, nullptr, &views[i]));
            frames[i] = std::make_unique<VKFramebuffer>(gpu->getDevice(), pass, size, size, 1, &views[i]);
        }
    }

    VkDescriptorImageInfo VKShadowPass::GetDepth() const {
        return {sampler, image->getImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
    }

    void VKShadowPass::Prepare(Renderer &renderer, float aspect) {
        sun = ShadowCascades::FindSun(renderer);
        if (!pass) CreatePipeline();
        if (!image || (sun && resolution == 1)) {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(gpu->getPhysicalDevice(), &properties);
            CreateTargets(sun ? std::min(ShadowCascades::Resolution, properties.limits.maxImageDimension2D) : 1u);
        }
        std::fill_n(uniforms.radiance, 4, 0.0f);
        if (!sun) return;
        auto *camera = renderer.getCamera();
        const auto view = camera->getViewMatrix();
        const auto direction = sun->getParent()->getWorldTransform().getPosition().normalise();
        const auto radiance = sun->getColour() * sun->getIntensity();
        cascades.Update(view, camera->getFov(), aspect, direction, resolution);
        for (unsigned i = 0; i < ShadowCascades::Count; ++i)
            std::copy_n(cascades.matrices[i].m, 16, uniforms.matrices[i]);
        std::copy_n(view.m, 16, uniforms.view);
        std::copy(cascades.splits.begin(), cascades.splits.end(), uniforms.splits);
        uniforms.direction[0] = direction.x;
        uniforms.direction[1] = direction.y;
        uniforms.direction[2] = direction.z;
        uniforms.radiance[0] = radiance.x;
        uniforms.radiance[1] = radiance.y;
        uniforms.radiance[2] = radiance.z;
        uniforms.settings[0] = __EngineConfig.rendererNearPlane;
        uniforms.settings[1] = ShadowCascades::BlendFraction;
        uniforms.settings[2] = __EngineConfig.shadowSunAngularRadius;
    }

    void VKShadowPass::Record(Renderer &renderer, VKCommandBuffer *buffer) {
        if (!sun && initialized) return;
        auto cmd = buffer->getCommandBuffer();
        VkRect2D area{{0, 0}, {resolution, resolution}};
        VkViewport viewport{0, 0, float(resolution), float(resolution), 0, 1};
        VkClearValue clear{};
        clear.depthStencil.depth = 1;
        for (unsigned i = 0; i < ShadowCascades::Count; ++i) {
            VkRenderPassBeginInfo begin{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            begin.renderPass = pass;
            begin.framebuffer = frames[i]->getFramebuffer();
            begin.renderArea = area;
            begin.clearValueCount = 1;
            begin.pClearValues = &clear;
            vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
            if (sun) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdSetViewport(cmd, 0, 1, &viewport);
                vkCmdSetScissor(cmd, 0, 1, &area);
                VertexBuffer *vertices = nullptr;
                auto bind = [&](Pipeline *surface) {
                    auto *graphics = static_cast<VKGraphicsPipeline *>(surface);
                    vertices = graphics->getTopology() == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
                                   ? graphics->getVertexBuffer()
                                   : nullptr;
                    if (vertices) vertices->Bind(buffer);
                };
                auto draw = [&](Pipeline *, RenderableComponent *object) {
                    if (!vertices) return;
                    const auto transform = cascades.matrices[i] * object->getParent()->getWorldTransform().
                                           getTransformationMatrix();
                    vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, transform.m);
                    for (unsigned sub = 0; sub < vertices->getSubmeshCount(); ++sub)
                        vertices->Draw(buffer, sub);
                };
                for (const auto *batches: {&renderer.getModels3D(), &renderer.getModelsTransparency()})
                    Renderer::VisitBatches(*batches, bind, [](Pipeline *, Material *) {
                                           }, draw, [](Pipeline *) {
                                           });
            }
            vkCmdEndRenderPass(cmd);
        }
        initialized = true;
    }

    VKShadowPass::~VKShadowPass() {
        ReleaseTargets();
        vkDestroyPipeline(gpu->getDevice(), pipeline, nullptr);
        vkDestroyPipelineLayout(gpu->getDevice(), layout, nullptr);
        vkDestroyRenderPass(gpu->getDevice(), pass, nullptr);
        vkDestroySampler(gpu->getDevice(), sampler, nullptr);
    }
}