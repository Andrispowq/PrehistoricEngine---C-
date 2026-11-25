#include "Includes.hpp"
#include "VKEnvironmentLighting.h"
#include "VKAtmosphereParameters.h"

#include "platform/vulkan/texture/VKImageMipmaps.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

namespace Prehistoric {
    namespace {
        void Check(VkResult result) {
            if (result != VK_SUCCESS)
                throw std::runtime_error("Vulkan IBL operation failed: " + std::to_string(result));
        }

        struct BakeResources {
            VkDevice device;
            VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
            VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
            VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
            VkShaderModule module = VK_NULL_HANDLE;
            VkPipeline conversionPipeline = VK_NULL_HANDLE;
            VkShaderModule conversionModule = VK_NULL_HANDLE;
            VkPipeline pipeline = VK_NULL_HANDLE;
            VkCommandPool commandPool = VK_NULL_HANDLE;
            VkFence fence = VK_NULL_HANDLE;

            ~BakeResources() {
                vkDestroyFence(device, fence, nullptr);
                vkDestroyCommandPool(device, commandPool, nullptr);
                vkDestroyPipeline(device, conversionPipeline, nullptr);
                vkDestroyShaderModule(device, conversionModule, nullptr);
                vkDestroyPipeline(device, pipeline, nullptr);
                vkDestroyShaderModule(device, module, nullptr);
                vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
                vkDestroyDescriptorPool(device, descriptorPool, nullptr);
                vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
            }
        };
    }

    VKEnvironmentLighting::VKEnvironmentLighting(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue,
                                                 uint32_t queueFamily, VkDescriptorImageInfo source,
                                                 uint32_t irradianceSize, uint32_t prefilterSize, uint32_t levels,
                                                 uint32_t cubeSize, const std::vector<char> &conversion,
                                                 const std::vector<char> &shader, uint32_t lutSize, uint32_t samples,
                                                 const AtmosphereParameters *sky, VKEnvironmentLighting *reuseLutFrom,
                                                 bool includeSunDisk)
        : physicalDevice(physicalDevice), device(device), lutSamples(samples) {
        if (!cubeSize || !irradianceSize || !prefilterSize || !levels || !lutSize || !samples)
            throw std::invalid_argument("Vulkan IBL dimensions and sample count must be positive");

        VkFormatProperties properties{};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R32G32B32A32_SFLOAT, &properties);
        const auto required = VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
        if ((properties.optimalTilingFeatures & required) != required)
            throw std::runtime_error("Vulkan IBL requires sampled/storage RGBA32F images");

        try {
            levels = (std::min)(levels, uint32_t(std::floor(std::log2(prefilterSize))) + 1);
            const auto blitFeatures = VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                                      VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
            uint32_t cubeLevels = (properties.optimalTilingFeatures & blitFeatures) == blitFeatures
                                      ? uint32_t(std::floor(std::log2(cubeSize))) + 1
                                      : 1;
            CreateMap(environment, cubeSize, cubeLevels, 6);
            CreateMap(irradiance, irradianceSize, 1, 6);
            CreateMap(prefilter, prefilterSize, levels, 6);
            bool reuseLut = reuseLutFrom && reuseLutFrom->device == device && reuseLutFrom->brdf.size == lutSize &&
                            reuseLutFrom->lutSamples == samples;
            if (!reuseLut) CreateMap(brdf, lutSize, 1, 1);

            VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
            bool linear = properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
            samplerInfo.magFilter = samplerInfo.minFilter = linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
            samplerInfo.mipmapMode = linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
            samplerInfo.addressModeU = samplerInfo.addressModeV =
                                       samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.maxLod = float((std::max)(levels, environment.levels) - 1);
            Check(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));
            Bake(queue, queueFamily, source, conversion, shader, samples, sky, includeSunDisk);
            if (reuseLut) std::swap(brdf, reuseLutFrom->brdf);
        } catch (...) {
            vkDestroySampler(device, sampler, nullptr);
            DestroyMap(brdf);
            DestroyMap(prefilter);
            DestroyMap(irradiance);
            DestroyMap(environment);
            throw;
        }
    }

    VKEnvironmentLighting::~VKEnvironmentLighting() {
        vkDestroySampler(device, sampler, nullptr);
        DestroyMap(brdf);
        DestroyMap(prefilter);
        DestroyMap(irradiance);
        DestroyMap(environment);
    }

    void VKEnvironmentLighting::CreateMap(Map &map, uint32_t size, uint32_t levels, uint32_t layers) {
        map.size = size;
        map.levels = levels;
        map.layers = layers;

        VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.flags = layers == 6 ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        imageInfo.extent = {size, size, 1};
        imageInfo.mipLevels = levels;
        imageInfo.arrayLayers = layers;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        if (&map == &environment) imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        Check(vkCreateImage(device, &imageInfo, nullptr, &map.image));

        VkMemoryRequirements requirements{};
        vkGetImageMemoryRequirements(device, map.image, &requirements);
        VkPhysicalDeviceMemoryProperties memoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
        uint32_t memoryType = UINT32_MAX;
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
            if ((requirements.memoryTypeBits & (1u << i)) &&
                (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
                memoryType = i;
                break;
            }
        }

        if (memoryType == UINT32_MAX) {
            throw std::runtime_error("No device-local memory for Vulkan IBL");
        }

        VkMemoryAllocateInfo allocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocation.allocationSize = requirements.size;
        allocation.memoryTypeIndex = memoryType;
        Check(vkAllocateMemory(device, &allocation, nullptr, &map.memory));
        Check(vkBindImageMemory(device, map.image, map.memory, 0));

        VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewInfo.image = map.image;
        viewInfo.viewType = layers == 6 ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = imageInfo.format;
        viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, levels, 0, layers};
        Check(vkCreateImageView(device, &viewInfo, nullptr, &map.view));
        map.storageViews.resize(levels, VK_NULL_HANDLE);

        for (uint32_t level = 0; level < levels; ++level) {
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            viewInfo.subresourceRange.baseMipLevel = level;
            viewInfo.subresourceRange.levelCount = 1;
            Check(vkCreateImageView(device, &viewInfo, nullptr, &map.storageViews[level]));
        }
    }

    void VKEnvironmentLighting::DestroyMap(Map &map) {
        for (auto view: map.storageViews) vkDestroyImageView(device, view, nullptr);
        vkDestroyImageView(device, map.view, nullptr);
        vkDestroyImage(device, map.image, nullptr);
        vkFreeMemory(device, map.memory, nullptr);
    }

    VkDescriptorImageInfo VKEnvironmentLighting::getEnvironment() const {
        return {sampler, environment.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkDescriptorImageInfo VKEnvironmentLighting::getIrradiance() const {
        return {sampler, irradiance.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkDescriptorImageInfo VKEnvironmentLighting::getPrefilter() const {
        return {sampler, prefilter.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkDescriptorImageInfo VKEnvironmentLighting::getBRDF() const {
        return {sampler, brdf.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    void VKEnvironmentLighting::Bake(VkQueue queue, uint32_t queueFamily, VkDescriptorImageInfo source,
                                     const std::vector<char> &conversion, const std::vector<char> &shader,
                                     uint32_t samples,
                                     const AtmosphereParameters *sky, bool includeSunDisk) {
        BakeResources resources{device};
        VkDescriptorSetLayoutBinding bindings[2]{};
        bindings[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
        bindings[1] = {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
        VkDescriptorSetLayoutCreateInfo setInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        setInfo.bindingCount = 2;
        setInfo.pBindings = bindings;
        Check(vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &resources.setLayout));

        struct Parameters {
            int mode;
            int resolution;
            float roughness;
            int samples;
        };
        static_assert(sizeof(Parameters) == 16);

        VkPushConstantRange push{
            VK_SHADER_STAGE_COMPUTE_BIT, 0,
            sky ? uint32_t(AtmospherePushOffset + sizeof(*sky)) : uint32_t(sizeof(Parameters))
        };
        VkPipelineLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &resources.setLayout;
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &push;
        Check(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &resources.pipelineLayout));

        VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        moduleInfo.codeSize = shader.size();
        moduleInfo.pCode = reinterpret_cast<const uint32_t *>(shader.data());
        Check(vkCreateShaderModule(device, &moduleInfo, nullptr, &resources.module));

        VkComputePipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipelineInfo.stage.module = resources.module;
        pipelineInfo.stage.pName = "main";
        pipelineInfo.layout = resources.pipelineLayout;
        Check(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resources.pipeline));

        moduleInfo.codeSize = conversion.size();
        moduleInfo.pCode = reinterpret_cast<const uint32_t *>(conversion.data());
        Check(vkCreateShaderModule(device, &moduleInfo, nullptr, &resources.conversionModule));
        pipelineInfo.stage.module = resources.conversionModule;
        Check(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                       &resources.conversionPipeline));

        uint32_t count = prefilter.levels + 3;
        VkDescriptorPoolSize poolSizes[] =
        {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, count}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, count}
        };

        VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        poolInfo.maxSets = count;
        poolInfo.poolSizeCount = 2;
        poolInfo.pPoolSizes = poolSizes;
        Check(vkCreateDescriptorPool(device, &poolInfo, nullptr, &resources.descriptorPool));

        std::vector<VkDescriptorSetLayout> layouts(count, resources.setLayout);
        std::vector<VkDescriptorSet> sets(count);
        VkDescriptorSetAllocateInfo allocation{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        allocation.descriptorPool = resources.descriptorPool;
        allocation.descriptorSetCount = count;
        allocation.pSetLayouts = layouts.data();
        Check(vkAllocateDescriptorSets(device, &allocation, sets.data()));

        VkCommandPoolCreateInfo commandPoolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        commandPoolInfo.queueFamilyIndex = queueFamily;
        Check(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &resources.commandPool));
        VkCommandBufferAllocateInfo commandInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        commandInfo.commandPool = resources.commandPool;
        commandInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandInfo.commandBufferCount = 1;

        VkCommandBuffer command;
        Check(vkAllocateCommandBuffers(device, &commandInfo, &command));
        VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        Check(vkBeginCommandBuffer(command, &begin));

        // The source upload completed on this same graphics/compute queue. Make
        // its transfer writes explicitly visible to the new compute consumer.
        VkMemoryBarrier sourceBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
        sourceBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             0, 1, &sourceBarrier, 0, nullptr, 0, nullptr);

        Map *maps[] = {&environment, &irradiance, &prefilter, &brdf};
        uint32_t setIndex = 0;
        VkDescriptorImageInfo cubeSource{sampler, environment.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        for (int mode = -1; mode < (brdf.image ? 3 : 2); ++mode) {
            bool converting = mode == -1;
            Map &map = *maps[mode + 1];
            vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE,
                              converting ? resources.conversionPipeline : resources.pipeline);
            VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = map.image;
            barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, map.levels, 0, map.layers};
            vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &barrier);

            for (uint32_t level = 0; level < (converting ? 1u : map.levels); ++level) {
                VkDescriptorImageInfo output{VK_NULL_HANDLE, map.storageViews[level], VK_IMAGE_LAYOUT_GENERAL};
                VkWriteDescriptorSet writes[2]{};
                for (uint32_t i = 0; i < 2; ++i) {
                    writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writes[i].dstSet = sets[setIndex];
                    writes[i].dstBinding = i;
                    writes[i].descriptorCount = 1;
                    writes[i].descriptorType = bindings[i].descriptorType;
                    writes[i].pImageInfo = i == 0 ? (converting ? &source : &cubeSource) : &output;
                }

                // Each mip owns its descriptor set; no recorded dispatch observes
                // a later update intended for another mip.
                uint32_t firstBinding = converting && sky ? 1 : 0;
                vkUpdateDescriptorSets(device, 2 - firstBinding, writes + firstBinding, 0, nullptr);
                vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_COMPUTE, resources.pipelineLayout,
                                        0, 1, &sets[setIndex++], 0, nullptr);
                uint32_t size = (std::max)(map.size >> level, 1u);
                Parameters parameters
                {
                    mode, int(size), map.levels > 1 ? float(level) / float(map.levels - 1) : 0.0f, int(samples)
                };
                if (converting && sky) {
                    const uint32_t includeDisk = includeSunDisk ? 1u : 0u;
                    vkCmdPushConstants(command, resources.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                                       0, sizeof(includeDisk), &includeDisk);
                    vkCmdPushConstants(command, resources.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                                       AtmospherePushOffset, sizeof(*sky), sky);
                } else {
                    vkCmdPushConstants(command, resources.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                       sizeof(parameters), &parameters);
                }
                vkCmdDispatch(command, (size + 7) / 8, (size + 7) / 8, map.layers);
            }

            if (converting && map.levels > 1) {
                barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     0, 0, nullptr, 0, nullptr, 1, &barrier);
                VKRecordImageMipmaps(command, map.image, map.size, map.size, map.levels, 6);
                continue;
            }
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }

        Check(vkEndCommandBuffer(command));
        VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        Check(vkCreateFence(device, &fenceInfo, nullptr, &resources.fence));
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &command;
        Check(vkQueueSubmit(queue, 1, &submit, resources.fence));
        Check(vkWaitForFences(device, 1, &resources.fence, VK_TRUE, UINT64_MAX));
    }
}