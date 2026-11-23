#include "Includes.hpp"
#include "VKEnvironmentBackground.h"
#include "VKAtmosphereParameters.h"
#include "prehistoric/core/modules/atmosphere/Atmosphere.h"
#include "platform/vulkan/framework/context/VKContext.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/rendering/renderpass/VKRenderpass.h"
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
            float rayX[4];
            float rayY[4];
            float rayCenter[4];
            AtmosphereParameters sky;
        };

        static_assert(offsetof(BackgroundConstants, sky) == AtmospherePushOffset);
        static_assert(sizeof(BackgroundConstants) == 120);

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

    VkDescriptorImageInfo VKEnvironmentBackground::LoadSource(Window* window)
    {
        const auto& location = __EnvironmentMapConfig.environmentMapLocation;
        if (location != sourceLocation)
        {
            texture.reset();
            vkDestroySampler(device->getDevice(), sampler, nullptr);
            sampler = VK_NULL_HANDLE;
            sourceLocation = location;
        }
        if (!texture && !location.empty())
        {
            texture.reset(static_cast<VKTexture*>(TextureLoader::LoadTexture(window, location, Bilinear, ClampToEdge)));

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
        }
        return 
        {
            sampler, texture ? texture->getImage()->getImageView() : VK_NULL_HANDLE,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
    }

    VKEnvironmentBackground::~VKEnvironmentBackground() 
    {
        vkDestroyPipeline(device->getDevice(), atmospherePipeline, nullptr);
        vkDestroyPipeline(device->getDevice(), pipeline, nullptr);
        vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
        vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device->getDevice(), descriptorLayout, nullptr);
        vkDestroySampler(device->getDevice(), sampler, nullptr);
    }

    void VKEnvironmentBackground::RecreatePipeline() 
    {
        vkDestroyPipeline(device->getDevice(), atmospherePipeline, nullptr);
        vkDestroyPipeline(device->getDevice(), pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;

        atmospherePipeline = swapchain->getRenderpass()->CreateFullscreenPipeline("vulkan/background/background_VS.spv",
            "vulkan/atmosphere/scattering_FS.spv", pipelineLayout, 0);
        pipeline = swapchain->getRenderpass()->CreateFullscreenPipeline("vulkan/background/background_VS.spv",
            "vulkan/background/background_FS.spv", pipelineLayout, 0);
    }

    void VKEnvironmentBackground::Draw(VkCommandBuffer commandBuffer, const Camera* camera, const Atmosphere* atmosphere) const
    {
        VkPipeline selected = atmosphere ? atmospherePipeline : pipeline;
        if (!atmosphere && !texture) return;
        // Remove translation from the actual view matrix, retaining camera rotation.
        Matrix4f view = camera->getViewMatrix();
        view.m[12] = view.m[13] = view.m[14] = 0.0f;
        Matrix4f inverse = (camera->getProjectionMatrix() * view).Invert();
        BackgroundConstants constants{};

        for (int axis = 0; axis < 3; ++axis)
        {
            constants.rayX[axis] = inverse.m[axis];
            constants.rayY[axis] = inverse.m[4 + axis];
            constants.rayCenter[axis] = inverse.m[8 + axis] + inverse.m[12 + axis];
        }
        if (atmosphere)
        {
            constants.sky = GetAtmosphereParameters(*atmosphere);
        }
        auto extent = swapchain->getSwapchainExtent();
        VkViewport viewport{0, 0, (float) extent.width, (float) extent.height, 0, 1};
        VkRect2D scissor{{0, 0}, extent};
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, selected);
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        if (!atmosphere)
        {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0,
                                    nullptr);
        }
        vkCmdPushConstants(commandBuffer, pipelineLayout, stages, 0, sizeof(constants), &constants);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }
}