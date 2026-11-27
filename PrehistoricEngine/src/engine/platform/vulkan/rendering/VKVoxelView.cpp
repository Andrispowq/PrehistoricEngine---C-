#include "Includes.hpp"
#include "VKVoxelView.h"
#include "VKRenderer.h"
#include "VKEnvironmentLighting.h"
#include "VKShadowPass.h"
#include "platform/vulkan/util/VKUtil.h"
#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"
#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric {
    namespace {
        void CheckVoxel(VkResult result) {
            if (result != VK_SUCCESS)
                throw std::runtime_error(
                    "Vulkan voxel view failed (" + std::to_string(result) + ")");
        }
    }

    size_t VKVoxelView::GridBytes(unsigned resolution, bool lighting) {
        const size_t count = size_t(resolution) * resolution * resolution;
        const size_t pyramid = (8 * count - 1) / 7;
        // Occupancy pyramid, surface and emission, isotropic and six directional
        // half-float radiance chains, plus accumulation and next-bounce grids.
        return sizeof(uint32_t) * ((lighting ? 15 * pyramid - 6 * count + 1 : count) + count);
    }

    void VKVoxelView::Mark(VkCommandBuffer cmd, Timing stage, bool end) {
        if (!timers) return;
        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timers, 2 * stage + int(end));
        if (end) recorded[stage] = true;
    }

    void VKVoxelView::StartTimings(VkCommandBuffer cmd, VKRenderer &renderer) {
        if (!timers) return;
        for (int i = 0; i < TimingCount; ++i) {
            if (!recorded[i]) continue;
            uint64_t timestamps[2]{};
            if (vkGetQueryPoolResults(device, timers, 2 * i, 2, sizeof(timestamps), timestamps, sizeof(uint64_t),
                                      VK_QUERY_RESULT_64_BIT) == VK_SUCCESS)
                renderer.voxelMilliseconds[i] = double((timestamps[1] - timestamps[0]) & timestampMask) * timestampMs;
            recorded[i] = false;
        }
        vkCmdResetQueryPool(cmd, timers, 0, 2 * TimingCount);
    }

    VKVoxelView::VKVoxelView(VKDevice *gpu, VKRenderpass &passes, VkRenderPass output, uint32_t resolution,
                             bool lighting)
        : resolution(resolution), lighting(lighting), gpu(gpu), device(gpu->getDevice()) {
        static_assert(Vertex::getNumberOfFloats() == 11);
        static_assert(sizeof(Surface) == 128);
        try {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(gpu->getPhysicalDevice(), &properties);
            uint32_t families = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(gpu->getPhysicalDevice(), &families, nullptr);
            std::vector<VkQueueFamilyProperties> queues(families);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu->getPhysicalDevice(), &families, queues.data());
            auto bits = queues[gpu->getQueueFamilyIndices().graphicsFamily].timestampValidBits;
            if (bits && lighting) {
                timestampMs = properties.limits.timestampPeriod * 1e-6;
                timestampMask = UINT64_MAX >> (64 - bits);
                VkQueryPoolCreateInfo create{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
                create.queryType = VK_QUERY_TYPE_TIMESTAMP;
                create.queryCount = 2 * TimingCount;
                CheckVoxel(vkCreateQueryPool(device, &create, nullptr, &timers));
            }
            cells = std::make_unique<VKBuffer>(
                gpu, GridBytes(resolution, lighting) - size_t(resolution) * resolution * resolution * 4,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            auto layout = [&](VkShaderStageFlags stage, uint32_t buffers, uint32_t images, uint32_t bytes,
                              VkDescriptorSetLayout &set,
                              VkPipelineLayout &pipeline) {
                const uint32_t count = buffers + images;
                VkDescriptorSetLayoutBinding bindings[9]{};
                for (uint32_t i = 0; i < count; ++i)
                    bindings[i] = {
                        i,
                        i >= buffers
                            ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                            : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        1, stage, nullptr
                    };
                VkDescriptorSetLayoutCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
                info.bindingCount = count;
                info.pBindings = bindings;
                CheckVoxel(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));
                VkPushConstantRange push{stage, 0, bytes};
                VkPipelineLayoutCreateInfo create{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
                create.setLayoutCount = create.pushConstantRangeCount = 1;
                create.pSetLayouts = &set;
                create.pPushConstantRanges = &push;
                CheckVoxel(vkCreatePipelineLayout(device, &create, nullptr, &pipeline));
            };
            layout(VK_SHADER_STAGE_COMPUTE_BIT, 3, 2, sizeof(Surface), bakeSet, bakeLayout);
            layout(VK_SHADER_STAGE_FRAGMENT_BIT, 1, lighting ? 8 : 0, 84, viewSet, viewLayout);
            auto compute = [&](const char *path, VkPipelineLayout layout, VkPipeline &out) {
                auto code = ResourceLoader::LoadShaderVK(path);
                VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
                moduleInfo.codeSize = code.size();
                moduleInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
                VkShaderModule module{};
                CheckVoxel(vkCreateShaderModule(device, &moduleInfo, nullptr, &module));
                VkComputePipelineCreateInfo pipeline{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
                pipeline.stage = {
                    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    nullptr,
                    0,
                    VK_SHADER_STAGE_COMPUTE_BIT,
                    module,
                    "main"
                };
                pipeline.layout = layout;
                auto result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline, nullptr, &out);
                vkDestroyShaderModule(device, module, nullptr);
                CheckVoxel(result);
            };
            compute("vulkan/voxel/occupancy.spv", bakeLayout, bakePipeline);
            scrollScratch = std::make_unique<VKBuffer>(gpu, size_t(resolution) * resolution * resolution * 4,
                                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            layout(VK_SHADER_STAGE_COMPUTE_BIT, 2, 0, 32, scrollSet, scrollLayout);
            compute("vulkan/voxel/scroll.spv", scrollLayout, scrollPipeline);
            VkDescriptorPoolSize scrollSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2};
            VkDescriptorPoolCreateInfo scrollCreate{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
            scrollCreate.maxSets = 1;
            scrollCreate.poolSizeCount = 1;
            scrollCreate.pPoolSizes = &scrollSize;
            CheckVoxel(vkCreateDescriptorPool(device, &scrollCreate, nullptr, &scrollPool));
            VkDescriptorSetAllocateInfo scrollAlloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
            scrollAlloc.descriptorPool = scrollPool;
            scrollAlloc.descriptorSetCount = 1;
            scrollAlloc.pSetLayouts = &scrollSet;
            CheckVoxel(vkAllocateDescriptorSets(device, &scrollAlloc, &scrollInputs));
            VkDescriptorBufferInfo scrollBuffers[] = {
                {cells->getBuffer(), 0, cells->getSize()},
                {scrollScratch->getBuffer(), 0, scrollScratch->getSize()}
            };
            for (unsigned i = 0; i < 2; ++i) {
                VkWriteDescriptorSet write{
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, scrollInputs, i, 0, 1,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &scrollBuffers[i]
                };
                vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
            }

            VkDescriptorSetLayoutBinding terrainBindings[] = {
                {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 12, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}
            };
            VkDescriptorSetLayoutCreateInfo terrainInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
            terrainInfo.bindingCount = 3;
            terrainInfo.pBindings = terrainBindings;
            CheckVoxel(vkCreateDescriptorSetLayout(device, &terrainInfo, nullptr, &terrainSet));
            VkPipelineLayoutCreateInfo terrainCreate{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
            terrainCreate.setLayoutCount = 1;
            terrainCreate.pSetLayouts = &terrainSet;
            CheckVoxel(vkCreatePipelineLayout(device, &terrainCreate, nullptr, &terrainLayout));
            compute("vulkan/voxel/terrain.spv", terrainLayout, terrainPipeline);

            if (lighting) {
                CreateRadianceTextures(gpu);
                layout(VK_SHADER_STAGE_COMPUTE_BIT, 1, 2, 112, lightSet, lightLayout);
                compute("vulkan/voxel/lighting.spv", lightLayout, lightPipeline);
            }
            viewPipeline = passes.CreateFullscreenPipeline("vulkan/postProcessing/hdr_VS.spv", "vulkan/voxel/view.spv",
                                                           viewLayout, output);
        } catch (...) {
            Release();
            throw;
        }
    }

    void VKVoxelView::CreateRadianceTextures(VKDevice *gpu) {
        VkFormatProperties format{};
        vkGetPhysicalDeviceFormatProperties(gpu->getPhysicalDevice(), VK_FORMAT_R16G16B16A16_SFLOAT, &format);
        VkImageFormatProperties image{};
        const auto result = vkGetPhysicalDeviceImageFormatProperties(gpu->getPhysicalDevice(),
                                                                     VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TYPE_3D,
                                                                     VK_IMAGE_TILING_OPTIMAL,
                                                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                     VK_IMAGE_USAGE_SAMPLED_BIT, 0, &image);
        if (result != VK_SUCCESS || !(format.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
            ||
            resolution > std::min({image.maxExtent.width, image.maxExtent.height, image.maxExtent.depth}))
            return;

        // Keep the exact half-float radiance and directional mip chains. Hardware
        // trilinear filtering replaces eight buffer loads per cone sample.
        for (uint32_t face = 0; face < radianceTextures.size(); ++face) {
            auto &texture = radianceTextures[face];
            const uint32_t size = face == 0 ? resolution : resolution / 2;
            for (uint32_t n = size; n; n >>= 1) ++texture.levels;
            VkImageCreateInfo image{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
            image.imageType = VK_IMAGE_TYPE_3D;
            image.format = VK_FORMAT_R16G16B16A16_SFLOAT;
            image.extent = {size, size, size};
            image.mipLevels = texture.levels;
            image.arrayLayers = 1;
            image.samples = VK_SAMPLE_COUNT_1_BIT;
            image.tiling = VK_IMAGE_TILING_OPTIMAL;
            image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            CheckVoxel(vkCreateImage(device, &image, nullptr, &texture.image));
            VkMemoryRequirements requirements;
            vkGetImageMemoryRequirements(device, texture.image, &requirements);
            VkMemoryAllocateInfo allocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            allocation.allocationSize = requirements.size;
            allocation.memoryTypeIndex = VKUtil::FindMemoryType(gpu->getPhysicalDevice(), requirements.memoryTypeBits,
                                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            CheckVoxel(vkAllocateMemory(device, &allocation, nullptr, &texture.memory));
            CheckVoxel(vkBindImageMemory(device, texture.image, texture.memory, 0));
            VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            view.image = texture.image;
            view.viewType = VK_IMAGE_VIEW_TYPE_3D;
            view.format = image.format;
            view.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, texture.levels, 0, 1};
            CheckVoxel(vkCreateImageView(device, &view, nullptr, &texture.view));
        }
        VkSamplerCreateInfo sampler{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        sampler.magFilter = sampler.minFilter = VK_FILTER_LINEAR;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler.addressModeU = sampler.addressModeV = sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler.maxLod = float(radianceTextures[0].levels - 1);
        CheckVoxel(vkCreateSampler(device, &sampler, nullptr, &radianceSampler));
    }

    void VKVoxelView::UploadRadiance(VkCommandBuffer cmd) {
        if (!radianceSampler) return;
        const size_t volume = size_t(resolution) * resolution * resolution;
        const size_t pyramid = (8 * volume - 1) / 7;
        const size_t radiance = (pyramid + 2 * volume + 1) & ~size_t(1);
        VkMemoryBarrier ready{
            VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT
        };
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 1, &ready, 0, nullptr, 0, nullptr);
        for (uint32_t face = 0; face < radianceTextures.size(); ++face) {
            const auto &texture = radianceTextures[face];
            VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            barrier.srcAccessMask = radianceUploaded ? VK_ACCESS_SHADER_READ_BIT : 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.oldLayout = radianceUploaded ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = texture.image;
            barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, texture.levels, 0, 1};
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &barrier);
            std::vector<VkBufferImageCopy> copies;
            size_t offset = face == 0 ? radiance : radiance + 2 * (pyramid + (face - 1) * (pyramid - volume));
            uint32_t size = face == 0 ? resolution : resolution / 2;
            for (uint32_t level = 0; level < texture.levels; ++level, size >>= 1) {
                VkBufferImageCopy copy{};
                copy.bufferOffset = offset * sizeof(uint32_t);
                copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 1};
                copy.imageExtent = {size, size, size};
                copies.push_back(copy);
                offset += 2 * size_t(size) * size * size;
            }
            vkCmdCopyBufferToImage(cmd, cells->getBuffer(), texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   uint32_t(copies.size()), copies.data());
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }
        radianceUploaded = true;
    }

    void VKVoxelView::Release() {
        vkDestroyDescriptorPool(device, scrollPool, nullptr);
        vkDestroyPipeline(device, scrollPipeline, nullptr);
        vkDestroyPipelineLayout(device, scrollLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, scrollSet, nullptr);
        for (const auto &texture: radianceTextures) {
            vkDestroyImageView(device, texture.view, nullptr);
            vkDestroyImage(device, texture.image, nullptr);
            vkFreeMemory(device, texture.memory, nullptr);
        }
        vkDestroySampler(device, radianceSampler, nullptr);
        vkDestroyQueryPool(device, timers, nullptr);
        vkDestroyDescriptorPool(device, pool, nullptr);
        vkDestroyPipeline(device, lightPipeline, nullptr);
        vkDestroyPipelineLayout(device, lightLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, lightSet, nullptr);
        vkDestroyPipeline(device, terrainPipeline, nullptr);
        vkDestroyPipelineLayout(device, terrainLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, terrainSet, nullptr);
        vkDestroyPipeline(device, bakePipeline, nullptr);
        vkDestroyPipeline(device, viewPipeline, nullptr);
        vkDestroyPipelineLayout(device, bakeLayout, nullptr);
        vkDestroyPipelineLayout(device, viewLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, bakeSet, nullptr);
        vkDestroyDescriptorSetLayout(device, viewSet, nullptr);
    }

    VKVoxelView::~VKVoxelView() { Release(); }

    bool VKVoxelView::Draw::operator==(const Draw &other) const {
        return vertices == other.vertices && indices == other.indices && albedo == other.albedo &&
               sampler == other.sampler && surface.model == other.surface.model &&
               surface.colour == other.surface.colour && metallic == other.metallic &&
               metallicSampler == other.metallicSampler && surface.material == other.surface.material &&
               surface.range == other.surface.range;
    }

    std::vector<VKVoxelView::Draw> VKVoxelView::Collect(VKRenderer &renderer, const Vec4 &grid) const {
        std::vector<Draw> draws;
        for (const auto &[pipeline, batches]: renderer.getModels3D()) {
            auto *graphics = static_cast<VKGraphicsPipeline *>(pipeline);
            if (graphics->getTopology() != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ||
                graphics->getVertexBuffer()->getType() != VertexBufferType::MESH)
                continue;
            auto *mesh = static_cast<VKMeshVertexBuffer *>(graphics->getVertexBuffer());
            for (const auto &[fallback, objects]: batches)
                for (const auto *object: objects) {
                    const auto transform = object->getParent()->getWorldTransform().getTransformationMatrix();
                    for (uint32_t i = 0; i < mesh->getSubmeshCount(); ++i) {
                        auto *material = object->ResolveSubmeshMaterial(mesh->getModel(), i, fallback);
                        if (!material || mesh->IndexCount(i) < 3) continue;
                        auto colour = material->getVector3f(COLOUR);
                        bool mapped = colour.x < 0 && material->exists(ALBEDO_MAP);
                        auto *tex = static_cast<VKTexture *>(material->getTexture(ALBEDO_MAP));
                        if (colour.x < 0) colour = Vector3f(1);
                        Draw draw{};
                        draw.vertices = mesh->GetVertexBuffer();
                        draw.indices = mesh->GetIndexBuffer();
                        draw.albedo = tex->getImage()->getImageView();
                        draw.sampler = tex->getSampler();
                        std::copy_n(transform.m, 16, draw.surface.model.begin());
                        draw.surface.grid = grid;
                        auto *metal = static_cast<VKTexture *>(material->getTexture(MROT_MAP));
                        draw.metallic = metal->getImage()->getImageView();
                        draw.metallicSampler = metal->getSampler();
                        draw.surface.colour = {
                            colour.x, colour.y, colour.z,
                            std::max(0.0f, material->getFloat(EMISSION))
                        };
                        draw.surface.material = {material->getVector4f(MROT).x, 0, 0, 0};
                        draw.surface.range = {
                            mesh->FirstIndex(i), mesh->IndexCount(i) / 3, resolution,
                            uint32_t(mapped) | (lighting ? 2u : 0u)
                        };
                        draws.push_back(draw);
                    }
                }
        }
        return draws;
    }

    void VKVoxelView::Bake(VkCommandBuffer cmd, const std::vector<Draw> &draws,
                           const std::vector<TerrainVoxelInput> &terrains, const VoxelScroll &scroll) {
        const uint32_t count = static_cast<uint32_t>(draws.size());
        VkDescriptorBufferInfo grid{cells->getBuffer(), 0, cells->getSize()};
        const bool reuse = pool && previousTerrains.size() == terrains.size() &&
                           std::equal(previousTerrains.begin(), previousTerrains.end(), terrains.begin(),
                                      [](const auto &a, const auto &b) {
                                          return a.textures == b.textures && a.textureIds == b.textureIds;
                                      }) &&
                           previous.size() == draws.size() &&
                           std::equal(previous.begin(), previous.end(), draws.begin(),
                                      [](const Draw &a, const Draw &b) {
                                          return a.vertices == b.vertices && a.indices == b.indices &&
                                                 a.albedo == b.albedo && a.sampler == b.sampler &&
                                                 a.metallic == b.metallic && a.metallicSampler == b.metallicSampler;
                                      });
        if (!reuse) {
            if (pool) {
                CheckVoxel(vkDeviceWaitIdle(device));
                vkDestroyDescriptorPool(device, pool, nullptr);
                pool = VK_NULL_HANDLE;
            }
            VkDescriptorPoolSize sizes[] = {
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 * count + 2 + 2 * uint32_t(terrains.size())},
                {
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    2 * count + 3 + (lighting ? 7u : 0u) + 12 * uint32_t(terrains.size())
                }
            };
            VkDescriptorPoolCreateInfo create{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
            create.maxSets = count + 1 + uint32_t(lighting) + uint32_t(terrains.size());
            create.poolSizeCount = 2;
            create.pPoolSizes = sizes;
            CheckVoxel(vkCreateDescriptorPool(device, &create, nullptr, &pool));
            std::vector<VkDescriptorSetLayout> layouts(count + 1, bakeSet);
            layouts.back() = viewSet;
            if (lighting) layouts.push_back(lightSet);
            layouts.insert(layouts.end(), terrains.size(), terrainSet);
            sets.resize(layouts.size());
            VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
            alloc.descriptorPool = pool;
            alloc.descriptorSetCount = uint32_t(layouts.size());
            alloc.pSetLayouts = layouts.data();
            CheckVoxel(vkAllocateDescriptorSets(device, &alloc, sets.data()));
            viewInputs = sets[count];
            VkWriteDescriptorSet gridWrite{
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, viewInputs, 0, 0, 1,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &grid
            };
            vkUpdateDescriptorSets(device, 1, &gridWrite, 0, nullptr);
            if (lighting) {
                std::array<VkDescriptorImageInfo, 7> images{};
                std::array<VkWriteDescriptorSet, 7> writes{};
                for (uint32_t i = 0; i < images.size(); ++i) {
                    images[i] = {radianceSampler, radianceTextures[i].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
                    writes[i] = {
                        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, viewInputs, 2 + i, 0, 1,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &images[i]
                    };
                }
                if (radianceSampler)
                    vkUpdateDescriptorSets(device, uint32_t(writes.size()), writes.data(), 0, nullptr);
                gridWrite.dstSet = lightInputs = sets[count + 1];
                vkUpdateDescriptorSets(device, 1, &gridWrite, 0, nullptr);
            }
            for (uint32_t i = 0; i < count; ++i) {
                const auto &draw = draws[i];
                VkDescriptorBufferInfo buffers[] = {
                    grid, {draw.vertices, 0, VK_WHOLE_SIZE}, {draw.indices, 0, VK_WHOLE_SIZE}
                };
                VkWriteDescriptorSet writes[5]{};
                for (uint32_t j = 0; j < 3; ++j)
                    writes[j] = {
                        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, sets[i], j, 0, 1,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &buffers[j]
                    };
                VkDescriptorImageInfo image{draw.sampler, draw.albedo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
                writes[3] = {
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, sets[i], 3, 0, 1,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &image
                };
                VkDescriptorImageInfo metal{
                    draw.metallicSampler, draw.metallic,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                };
                writes[4] = writes[3];
                writes[4].dstBinding = 4;
                writes[4].pImageInfo = &metal;
                vkUpdateDescriptorSets(device, 5, writes, 0, nullptr);
            }
        }
        if (!reuse) {
            terrainParameters.clear();
            for (size_t i = 0; i < terrains.size(); ++i) {
                auto params = std::make_unique<VKBuffer>(gpu, sizeof(TerrainVoxelParameters),
                                                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                VkDescriptorBufferInfo buffers[] = {grid, {params->getBuffer(), 0, params->getSize()}};
                const auto set = sets[count + 1 + uint32_t(lighting) + i];
                VkDescriptorImageInfo images[12]{};
                for (unsigned j = 0; j < 12; ++j) {
                    auto *texture = static_cast<VKTexture *>(terrains[i].textures[j]);
                    images[j] = {
                        texture->getSampler(), texture->getImage()->getImageView(),
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    };
                }
                VkWriteDescriptorSet writes[] = {
                    {
                        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, 0, 0, 1,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &buffers[0]
                    },
                    {
                        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, 1, 0, 1,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &buffers[1]
                    },
                    {
                        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, 2, 0, 12,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, images
                    }
                };
                vkUpdateDescriptorSets(device, 3, writes, 0, nullptr);
                terrainParameters.push_back(std::move(params));
            }
        }
        // A previous frame may still be tracing this grid when the scene changes.
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr, 0, nullptr);
        if (scroll.incremental) ScrollGeometry(cmd, scroll);
        else vkCmdFillBuffer(cmd, grid.buffer, 0, grid.range, 0);
        for (size_t i = 0; i < terrains.size(); ++i)
            vkCmdUpdateBuffer(cmd, terrainParameters[i]->getBuffer(), 0, sizeof(TerrainVoxelParameters),
                              &terrains[i].parameters);
        VkMemoryBarrier clear{
            VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
        };
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &clear, 0,
                             nullptr, 0, nullptr);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, bakePipeline);
        for (uint32_t i = 0; i < count; ++i) {
            const auto &draw = draws[i];
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, bakeLayout, 0, 1, &sets[i], 0, nullptr);
            auto params = draw.surface;
            if (scroll.incremental) {
                params.range[3] |= 4u;
                for (unsigned axis = 0; axis < 3; ++axis) params.material[axis + 1] = float(scroll.delta[axis]);
            }
            while (params.range[1]) {
                const auto remaining = params.range[1];
                params.range[1] = std::min(remaining, 65535u * 64);
                vkCmdPushConstants(cmd, bakeLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(params), &params);
                vkCmdDispatch(cmd, (params.range[1] + 63) / 64, 1, 1);
                params.range[0] += params.range[1] * 3;
                params.range[1] = remaining - params.range[1];
            }
        }
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, terrainPipeline);
        for (size_t i = 0; i < terrains.size(); ++i) {
            const auto set = sets[count + 1 + uint32_t(lighting) + i];
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, terrainLayout, 0, 1, &set, 0, nullptr);
            const auto &region = terrains[i].parameters.region;
            vkCmdDispatch(cmd, (uint32_t(region[2]) + 7) / 8, (uint32_t(region[3]) + 7) / 8, 1);
        }
        VkMemoryBarrier ready{
            VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT
        };
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &ready,
                             0, nullptr, 0, nullptr);
    }

    void VKVoxelView::ScrollGeometry(VkCommandBuffer cmd, const VoxelScroll &scroll) {
        const uint32_t count = resolution * resolution * resolution;
        const uint32_t pyramid = (8 * count - 1) / 7;
        const uint32_t offsets[] = {0, pyramid, pyramid + count};
        auto barrier = [&](VkPipelineStageFlags source, VkPipelineStageFlags destination,
                           VkAccessFlags writes, VkAccessFlags reads) {
            VkMemoryBarrier memory{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, writes, reads};
            vkCmdPipelineBarrier(cmd, source, destination, 0, 1, &memory, 0, nullptr, 0, nullptr);
        };
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, scrollPipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, scrollLayout, 0, 1, &scrollInputs, 0, nullptr);
        for (unsigned layer = 0; layer < (lighting ? 3u : 1u); ++layer) {
            barrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT);
            VkBufferCopy copy{VkDeviceSize(offsets[layer]) * 4, 0, VkDeviceSize(count) * 4};
            vkCmdCopyBuffer(cmd, cells->getBuffer(), scrollScratch->getBuffer(), 1, &copy);
            barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
            int params[8] = {
                scroll.delta[0], scroll.delta[1], scroll.delta[2], int(resolution), int(offsets[layer]), 0, 0, 0
            };
            vkCmdPushConstants(cmd, scrollLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(params), params);
            vkCmdDispatch(cmd, (resolution + 3) / 4, (resolution + 3) / 4, (resolution + 3) / 4);
        }
        barrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
    }

    void VKVoxelView::Prepare(VkCommandBuffer cmd, VKRenderer &renderer) {
        const auto width = renderer.voxelExtent;
        const auto min = renderer.voxelCenter - Vector3f(width * 0.5f);
        const Vec4 grid{min.x, min.y, min.z, width};
        StartTimings(cmd, renderer);
        auto draws = Collect(renderer, grid);
        auto terrains = CollectTerrainVoxels(renderer, grid, resolution, lighting);
        const bool sameGeometry = pool && previous == draws && previousTerrains == terrains;
        const bool geometryChanged = !sameGeometry || previousGrid != grid;
        VoxelScroll scroll;
        if (sameGeometry && terrains.empty()) scroll = PlanVoxelScroll(previousGrid, grid, resolution);
        if (geometryChanged) {
            Mark(cmd, Geometry, false);
            Bake(cmd, draws, terrains, scroll);
            renderer.voxelUpdate.mode = scroll.incremental
                                            ? Renderer::VoxelUpdate::Scrolled
                                            : Renderer::VoxelUpdate::Rebuilt;
            renderer.voxelUpdate.dirtyCells = scroll.incremental
                                                  ? scroll.dirtyCells
                                                  : uint64_t(resolution) * resolution * resolution;
            previousGrid = grid;
            Mark(cmd, Geometry, true);
            previous = std::move(draws);
            previousTerrains = std::move(terrains);
        }
        if (!lighting) return;
        const auto &sun = renderer.GetShadows().GetUniforms();
        Vec4 direction{}, radiance{};
        std::copy_n(sun.direction, 4, direction.begin());
        std::copy_n(sun.radiance, 4, radiance.begin());
        std::vector<std::pair<VkBuffer, std::array<float, 16> > > casters;
        const bool sunActive = radiance[0] > 0 || radiance[1] > 0 || radiance[2] > 0;
        if (!renderer.voxelSunShadows && sunActive) {
            for (const auto &[pipeline, batches]: renderer.getModelsTransparency()) {
                auto *graphics = static_cast<VKGraphicsPipeline *>(pipeline);
                if (graphics->getTopology() != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ||
                    graphics->getVertexBuffer()->getType() != VertexBufferType::MESH)
                    continue;
                auto mesh = static_cast<VKMeshVertexBuffer *>(graphics->getVertexBuffer())->GetVertexBuffer();
                for (const auto &[material, objects]: batches)
                    for (auto *object: objects) {
                        auto matrix = object->getParent()->getWorldTransform().getTransformationMatrix();
                        auto &entry = casters.emplace_back();
                        entry.first = mesh;
                        std::copy_n(matrix.m, 16, entry.second.begin());
                    }
            }
        }
        const auto sky = renderer.getEnvironmentLighting()->getEnvironment();
        int depth = renderer.voxelGI ? renderer.voxelBounces : 0;
        if (!geometryChanged && sky.imageView == previousEnvironment && depth == previousBounces && casters ==
            previousCasters &&
            renderer.voxelSunShadows == previousVoxelShadows && direction == previousDirection &&
            radiance == previousSun && renderer.voxelDirectional == previousDirectional)
            return;
        previousEnvironment = sky.imageView;
        previousDirection = direction;
        previousSun = radiance;
        previousBounces = depth;
        previousCasters = std::move(casters);
        previousVoxelShadows = renderer.voxelSunShadows;
        Mark(cmd, Injection, false);
        const auto light = renderer.GetShadows().RecordVoxelShadow(renderer, cmd, renderer.voxelCenter, width,
                                                                   depth > 0 && sunActive && !renderer.voxelSunShadows);
        const auto lightFromCell =
                light * Matrix4f::Translation(min) * Matrix4f::Scaling(Vector3f(width / float(resolution)));
        auto shadow = renderer.GetShadows().VoxelDepth();
        VkWriteDescriptorSet shadowWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, lightInputs, 1, 0, 1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &shadow
        };
        VkWriteDescriptorSet inputs[] = {shadowWrite, shadowWrite, shadowWrite};
        inputs[1].dstBinding = 2;
        inputs[1].pImageInfo = &sky;
        inputs[2].dstSet = viewInputs;
        inputs[2].pImageInfo = &sky;
        vkUpdateDescriptorSets(device, std::size(inputs), inputs, 0, nullptr);
        previousDirectional = renderer.voxelDirectional;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, lightPipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, lightLayout, 0, 1, &lightInputs, 0, nullptr);
        auto dispatch = [&](int stage, int level, bool skyBounce = false) {
            struct {
                Vec4 direction, radiance;
                int settings[4];
                float light[16];
            } params{
                direction,
                radiance,
                {
                    int(resolution), stage, level,
                    int(previousDirectional) | (int(previousVoxelShadows) << 1) | (int(skyBounce) << 2) | (
                        int(!previousTerrains.empty()) << 3)
                }
            };
            std::copy_n(lightFromCell.m, 16, params.light);
            vkCmdPushConstants(cmd, lightLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(params), &params);
            uint32_t groups = ((resolution >> level) + 3) / 4;
            vkCmdDispatch(cmd, groups, groups, groups);
            VkMemoryBarrier barrier{
                VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
            };
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1,
                                 &barrier, 0, nullptr, 0, nullptr);
        };
        auto pyramid = [&](int stage) {
            for (int level = 1; (resolution >> level) > 0; ++level)
                dispatch(stage, level);
        };
        if (geometryChanged) pyramid(0);
        if (!depth) {
            Mark(cmd, Injection, true);
            return;
        }
        dispatch(1, 0); // Direct radiance and accumulation base.
        Mark(cmd, Injection, true);
        Mark(cmd, Bounces, false);
        for (int bounce = 1; bounce < previousBounces; ++bounce) {
            pyramid(2);
            dispatch(3, 0, bounce == 1);
            dispatch(bounce + 1 == previousBounces ? 5 : 4, 0);
        }
        Mark(cmd, Bounces, true);
        Mark(cmd, Mips, false);
        pyramid(2);
        UploadRadiance(cmd);
        Mark(cmd, Mips, true);
    }

    void VKVoxelView::Render(VkCommandBuffer cmd, VKRenderer &renderer, VkRenderPass pass, VkFramebuffer frame,
                             VkExtent2D extent) {
        const auto width = renderer.voxelExtent;
        const auto min = renderer.voxelCenter - Vector3f(width * 0.5f);
        const Vec4 grid{min.x, min.y, min.z, width};
        Prepare(cmd, renderer);
        VkRenderPassBeginInfo begin{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        begin.renderPass = pass;
        begin.framebuffer = frame;
        begin.renderArea.extent = extent;
        vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
        VkViewport viewport{0, 0, float(extent.width), float(extent.height), 0, 1};
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &begin.renderArea);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, viewPipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, viewLayout, 0, 1, &viewInputs, 0, nullptr);
        struct {
            float inverse[16];
            Vec4 grid;
            uint32_t size;
        } params{{}, grid, resolution};
        const auto inverse = renderer.getCamera()->getViewProjectionMatrix().Invert();
        std::copy_n(inverse.m, 16, params.inverse);
        vkCmdPushConstants(cmd, viewLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(params), &params);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }
}