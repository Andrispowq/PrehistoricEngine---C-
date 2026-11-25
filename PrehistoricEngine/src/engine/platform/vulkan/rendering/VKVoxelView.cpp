#include "Includes.hpp"
#include "VKVoxelView.h"
#include "VKRenderer.h"
#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"
#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric {
    namespace {
        constexpr uint32_t Resolution = 128;

        void CheckVoxel(VkResult result) {
            if (result != VK_SUCCESS) throw std::runtime_error(
                "Vulkan voxel view failed (" + std::to_string(result) + ")");
        }
    }

    VKVoxelView::VKVoxelView(VKDevice *gpu, VKRenderpass &passes, VkRenderPass output)
        : device(gpu->getDevice()) {
        static_assert(Vertex::getNumberOfFloats() == 11);
        static_assert(sizeof(Surface) == 112);
        try {
            cells = std::make_unique<VKBuffer>(gpu, Resolution * Resolution * Resolution * sizeof(uint32_t),
                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            auto layout = [&](VkShaderStageFlags stage, uint32_t count, uint32_t bytes, VkDescriptorSetLayout &set,
                              VkPipelineLayout &pipeline) {
                VkDescriptorSetLayoutBinding bindings[4]{};
                for (uint32_t i = 0; i < count; ++i)
                    bindings[i] = {
                        i, i == 3 ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                        stage, nullptr
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
            layout(VK_SHADER_STAGE_COMPUTE_BIT, 4, sizeof(Surface), bakeSet, bakeLayout);
            layout(VK_SHADER_STAGE_FRAGMENT_BIT, 1, 84, viewSet, viewLayout);
            auto code = ResourceLoader::LoadShaderVK("vulkan/voxel/occupancy.spv");
            VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            moduleInfo.codeSize = code.size();
            moduleInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
            VkShaderModule module{};
            CheckVoxel(vkCreateShaderModule(device, &moduleInfo, nullptr, &module));
            VkComputePipelineCreateInfo pipeline{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
            pipeline.stage = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_COMPUTE_BIT, module,
                "main"
            };
            pipeline.layout = bakeLayout;
            auto result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline, nullptr, &bakePipeline);
            vkDestroyShaderModule(device, module, nullptr);
            CheckVoxel(result);
            viewPipeline = passes.CreateFullscreenPipeline("vulkan/postProcessing/hdr_VS.spv", "vulkan/voxel/view.spv",
                                                           viewLayout, output);
        } catch (...) {
            Release();
            throw;
        }
    }

    void VKVoxelView::Release() {
        vkDestroyDescriptorPool(device, pool, nullptr);
        vkDestroyPipeline(device, bakePipeline, nullptr);
        vkDestroyPipeline(device, viewPipeline, nullptr);
        vkDestroyPipelineLayout(device, bakeLayout, nullptr);
        vkDestroyPipelineLayout(device, viewLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, bakeSet, nullptr);
        vkDestroyDescriptorSetLayout(device, viewSet, nullptr);
    }

    VKVoxelView::~VKVoxelView() { Release(); }

    bool VKVoxelView::Draw::operator==(const Draw &other) const {
        return vertices == other.vertices && indices == other.indices && albedo == other.albedo && sampler == other.
               sampler
               && surface.model == other.surface.model && surface.colour == other.surface.colour
               && surface.grid == other.surface.grid && surface.range == other.surface.range;
    }

    std::vector<VKVoxelView::Draw> VKVoxelView::Collect(VKRenderer &renderer, const Vec4 &grid) const {
        std::vector<Draw> draws;
        for (const auto &[pipeline, batches]: renderer.getModels3D()) {
            auto *graphics = static_cast<VKGraphicsPipeline *>(pipeline);
            if (graphics->getTopology() != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ||
                graphics->getVertexBuffer()->getType() != VertexBufferType::MESH)
                continue;
            auto *mesh = static_cast<VKMeshVertexBuffer *>(graphics->getVertexBuffer());
            const auto materials = mesh->getModel().getMaterials();
            for (const auto &[fallback, objects]: batches)
                for (const auto *object: objects) {
                    const auto transform = object->getParent()->getWorldTransform().getTransformationMatrix();
                    for (uint32_t i = 0; i < mesh->getSubmeshCount(); ++i) {
                        const int index = mesh->getModel().getMesh(i).getMaterialIndex();
                        auto *material = index >= 0 && size_t(index) < materials.size() && materials[index]
                                             ? materials[index]
                                             : fallback;
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
                        draw.surface.colour = {colour.x, colour.y, colour.z, 1};
                        draw.surface.range = {
                            mesh->FirstIndex(i), mesh->IndexCount(i) / 3, Resolution, uint32_t(mapped)
                        };
                        draws.push_back(draw);
                    }
                }
        }
        return draws;
    }

    void VKVoxelView::Bake(VkCommandBuffer cmd, const std::vector<Draw> &draws) {
        const uint32_t count = static_cast<uint32_t>(draws.size());
        VkDescriptorBufferInfo grid{cells->getBuffer(), 0, cells->getSize()};
        const bool reuse = pool && previous.size() == draws.size() &&
                           std::equal(previous.begin(), previous.end(), draws.begin(),
                                      [](const Draw &a, const Draw &b) {
                                          return a.vertices == b.vertices && a.indices == b.indices && a.albedo == b.
                                                 albedo && a.sampler == b.sampler;
                                      });
        if (!reuse) {
            if (pool) {
                CheckVoxel(vkDeviceWaitIdle(device));
                vkDestroyDescriptorPool(device, pool, nullptr);
                pool = VK_NULL_HANDLE;
            }
            VkDescriptorPoolSize sizes[] = {
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 * count + 1},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, std::max(1u, count)}
            };
            VkDescriptorPoolCreateInfo create{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
            create.maxSets = count + 1;
            create.poolSizeCount = 2;
            create.pPoolSizes = sizes;
            CheckVoxel(vkCreateDescriptorPool(device, &create, nullptr, &pool));
            std::vector<VkDescriptorSetLayout> layouts(count + 1, bakeSet);
            layouts.back() = viewSet;
            sets.resize(count + 1);
            VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
            alloc.descriptorPool = pool;
            alloc.descriptorSetCount = count + 1;
            alloc.pSetLayouts = layouts.data();
            CheckVoxel(vkAllocateDescriptorSets(device, &alloc, sets.data()));
            viewInputs = sets.back();
            VkWriteDescriptorSet gridWrite{
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, viewInputs, 0, 0, 1,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &grid
            };
            vkUpdateDescriptorSets(device, 1, &gridWrite, 0, nullptr);
            for (uint32_t i = 0; i < count; ++i) {
                const auto &draw = draws[i];
                VkDescriptorBufferInfo buffers[] = {
                    grid, {draw.vertices, 0, VK_WHOLE_SIZE}, {draw.indices, 0, VK_WHOLE_SIZE}
                };
                VkWriteDescriptorSet writes[4]{};
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
                vkUpdateDescriptorSets(device, 4, writes, 0, nullptr);
            }
        }
        // A previous frame may still be tracing this grid when the scene changes.
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr, 0, nullptr);
        vkCmdFillBuffer(cmd, grid.buffer, 0, grid.range, 0);
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
            while (params.range[1]) {
                const auto remaining = params.range[1];
                params.range[1] = std::min(remaining, 65535u * 64);
                vkCmdPushConstants(cmd, bakeLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(params), &params);
                vkCmdDispatch(cmd, (params.range[1] + 63) / 64, 1, 1);
                params.range[0] += params.range[1] * 3;
                params.range[1] = remaining - params.range[1];
            }
        }
        VkMemoryBarrier ready{
            VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT
        };
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &ready, 0, nullptr, 0, nullptr);
    }

    void VKVoxelView::Render(VkCommandBuffer cmd, VKRenderer &renderer, VkRenderPass pass, VkFramebuffer frame,
                             VkExtent2D extent) {
        const auto width = renderer.voxelExtent;
        const auto min = renderer.voxelCenter - Vector3f(width * 0.5f);
        const Vec4 grid{min.x, min.y, min.z, width};
        auto draws = Collect(renderer, grid);
        if (!pool || previous != draws) {
            Bake(cmd, draws);
            previous = std::move(draws);
        }
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
        } params{{}, grid, Resolution};
        const auto inverse = renderer.getCamera()->getViewProjectionMatrix().Invert();
        std::copy_n(inverse.m, 16, params.inverse);
        vkCmdPushConstants(cmd, viewLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(params), &params);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }
}