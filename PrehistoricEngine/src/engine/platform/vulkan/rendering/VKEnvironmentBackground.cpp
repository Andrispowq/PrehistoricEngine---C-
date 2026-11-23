#include "Includes.hpp"
#include "VKEnvironmentBackground.h"
#include "platform/vulkan/framework/context/VKContext.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/rendering/renderpass/VKRenderpass.h"
#include "platform/vulkan/rendering/shaders/VKColourSpace.h"
#include "platform/vulkan/texture/VKTexture.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/core/resources/loaders/TextureLoader.h"
#include "prehistoric/common/rendering/shaders/Shader.h"

namespace Prehistoric 
{
    namespace 
    {
        constexpr VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        struct BackgroundConstants 
        {
            float inverseViewProjection[16];
            float exposure;
            float gamma;
        };

        static_assert(sizeof(BackgroundConstants) == 72);

        void Check(VkResult result) 
        {
            if (result != VK_SUCCESS)
                PR_LOG_RUNTIME_ERROR("Failed to create Vulkan environment background (VkResult %d)!\n", result);
        }
    }

    VKEnvironmentBackground::VKEnvironmentBackground(Window *window)
        : device((VKDevice *) window->getContext()->getDevices()),
          swapchain((VKSwapchain *) window->getSwapchain()) 
    {
        const auto &location = __EnvironmentMapConfig.environmentMapLocation;
        if (location.empty())return;
        texture.reset((VKTexture *) TextureLoader::LoadTexture(window, location, Bilinear, ClampToEdge));

        VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        VkFormatProperties properties{};
        vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), texture->getImage()->getImageFormat(),
                                            &properties);
        samplerInfo.magFilter = samplerInfo.minFilter =
                                properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
                                    ? VK_FILTER_LINEAR
                                    : VK_FILTER_NEAREST;
        samplerInfo.mipmapMode = samplerInfo.minFilter == VK_FILTER_LINEAR
                                     ? VK_SAMPLER_MIPMAP_MODE_LINEAR
                                     : VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.maxLod = float(texture->getImage()->getMipLevels() - 1);
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        Check(vkCreateSampler(device->getDevice(), &samplerInfo, nullptr, &sampler));

        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;
        Check(vkCreateDescriptorSetLayout(device->getDevice(), &layoutInfo, nullptr, &descriptorLayout));

        VkDescriptorPoolSize poolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
        VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        Check(vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &descriptorPool));

        VkDescriptorSetAllocateInfo allocation{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        allocation.descriptorPool = descriptorPool;
        allocation.descriptorSetCount = 1;
        allocation.pSetLayouts = &descriptorLayout;
        Check(vkAllocateDescriptorSets(device->getDevice(), &allocation, &descriptorSet));

        VkPushConstantRange constants{stages, 0, sizeof(BackgroundConstants)};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &constants;
        Check(vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

        RecreatePipeline();
    }

    void VKEnvironmentBackground::SetPrefilter(const VkDescriptorImageInfo& imageInfo)
    {
        VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        write.dstSet = descriptorSet;
        write.dstBinding = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device->getDevice(), 1, &write, 0, nullptr);
    }

    VkDescriptorImageInfo VKEnvironmentBackground::getSource() const
    {
        return 
        {
            sampler, texture ? texture->getImage()->getImageView() : VK_NULL_HANDLE,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
    }

    VKEnvironmentBackground::~VKEnvironmentBackground() 
    {
        vkDestroyPipeline(device->getDevice(), pipeline, nullptr);
        vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
        vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device->getDevice(), descriptorLayout, nullptr);
        vkDestroySampler(device->getDevice(), sampler, nullptr);
    }

    void VKEnvironmentBackground::RecreatePipeline() 
    {
        if (!texture) return;
        vkDestroyPipeline(device->getDevice(), pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;

        VkShaderModule modules[2]{};
        VkPipelineShaderStageCreateInfo shaderStages[2]{};
        const char *paths[] = {"vulkan/background/background_VS.spv", "vulkan/background/background_FS.spv"};

        for (size_t i = 0; i < 2; ++i) 
        {
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
        multisample.rasterizationSamples = device->getSampleCount();

        VkPipelineDepthStencilStateCreateInfo depth{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depth.depthTestEnable = VK_TRUE;
        depth.depthWriteEnable = VK_FALSE;
        depth.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        VkPipelineColorBlendAttachmentState attachment{};
        attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                    VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        blend.attachmentCount = 1;
        blend.pAttachments = &attachment;

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
        info.layout = pipelineLayout;
        info.renderPass = swapchain->getRenderpass()->getRenderPass();
        const VkResult result = vkCreateGraphicsPipelines(device->getDevice(), VK_NULL_HANDLE, 1, &info, nullptr,
                                                          &pipeline);

        for (auto module: modules)
        {
            vkDestroyShaderModule(device->getDevice(), module, nullptr);
        }
        Check(result);
    }

    void VKEnvironmentBackground::Draw(VkCommandBuffer commandBuffer, const Camera *camera) const 
    {
        if (!pipeline) return;
        // Remove translation from the actual view matrix, retaining camera rotation.
        Matrix4f view = camera->getViewMatrix();
        view.m[12] = view.m[13] = view.m[14] = 0.0f;
        Matrix4f inverse = (camera->getProjectionMatrix() * view).Invert();
        BackgroundConstants constants{};

        memcpy(constants.inverseViewProjection, inverse.m, sizeof(constants.inverseViewProjection));
        constants.exposure = __EngineConfig.rendererExposure;
        constants.gamma = VKPBRShaderGamma(swapchain->getColourImage()->getImageFormat(), __EngineConfig.rendererGamma);

        auto extent = swapchain->getSwapchainExtent();
        VkViewport viewport{0, 0, (float) extent.width, (float) extent.height, 0, 1};
        VkRect2D scissor{{0, 0}, extent};
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0,
                                nullptr);
        vkCmdPushConstants(commandBuffer, pipelineLayout, stages, 0, sizeof(constants), &constants);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }
}