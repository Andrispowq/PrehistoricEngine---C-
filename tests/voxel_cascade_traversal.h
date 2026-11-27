#pragma once
#include "tests/voxel_readback.h"
#include "prehistoric/common/rendering/VoxelCascades.h"
#include <fstream>

inline void ValidateCascadeTraversal(Prehistoric::Renderer &renderer) {
    using namespace Prehistoric;
    if (!dynamic_cast<VKRenderer *>(&renderer)) return;
    const char *shaderPath = std::getenv("PREHISTORIC_CASCADE_TEST_SHADER");
    if (!shaderPath) throw std::runtime_error("Missing cascade traversal test shader");
    auto *gpu = static_cast<VKDevice *>(renderer.getWindow()->getContext()->getDevices());
    const auto device = gpu->getDevice();
    vkDeviceWaitIdle(device);
    VoxelCascadeSnapshot snapshot;
    const auto bytes = MakeVoxelCascades(snapshot, 64, 4, {0, 0, 0}, 32, false);
    std::vector<uint32_t> data(bytes / 4, 0);
    std::memcpy(data.data(), &snapshot, sizeof(snapshot));
    for (unsigned c = 0; c < 4; ++c) {
        const auto &grid = snapshot.grids[c];
        for (unsigned i = 0; i < 2 * VoxelPyramid(64); ++i) data[grid[2] + i] = 0x3c003c00u;
    }
    auto occupy = [&](unsigned c, std::array<float, 3> position) {
        const auto &b = snapshot.bounds[c];
        const auto &g = snapshot.grids[c];
        std::array < unsigned, 3 > p{};
        for (unsigned a = 0; a < 3; ++a) p[a] = unsigned((position[a] - b[a]) * 64 / b[3]);
        unsigned offset = g[1];
        for (unsigned n = 64; n; n >>= 1) {
            data[offset + p[0] + n * (p[1] + n * p[2])] = 1;
            offset += n * n * n;
            for (auto &axis: p) axis >>= 1;
        }
    };
    occupy(0, {0, 2, 0});
    occupy(1, {20, 0, 0});
    occupy(3, {96, 0, 0});
    VKBuffer input(gpu, bytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VKBuffer output(gpu, 24, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto check = [](VkResult result) {
        if (result != VK_SUCCESS) throw std::runtime_error("Cascade compute API failure");
    };
    void *mapped = nullptr;
    check(vkMapMemory(device, input.getMemory(), 0, bytes, 0, &mapped));
    std::memcpy(mapped, data.data(), bytes);
    vkUnmapMemory(device, input.getMemory());
    struct Resources {
        VkDevice device;
        VkDescriptorSetLayout set{};
        VkDescriptorPool pool{};
        VkPipelineLayout layout{};
        VkShaderModule module{};
        VkPipeline pipeline{};

        ~Resources() {
            vkDestroyPipeline(device, pipeline, nullptr);
            vkDestroyShaderModule(device, module, nullptr);
            vkDestroyPipelineLayout(device, layout, nullptr);
            vkDestroyDescriptorPool(device, pool, nullptr);
            vkDestroyDescriptorSetLayout(device, set, nullptr);
        }
    } resources{device};
    VkDescriptorSetLayoutBinding bindings[] = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}
    };
    VkDescriptorSetLayoutCreateInfo setInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    setInfo.bindingCount = 2;
    setInfo.pBindings = bindings;
    check(vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &resources.set));
    VkDescriptorPoolSize poolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2};
    VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    check(vkCreateDescriptorPool(device, &poolInfo, nullptr, &resources.pool));
    VkDescriptorSetAllocateInfo allocate{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocate.descriptorPool = resources.pool;
    allocate.descriptorSetCount = 1;
    allocate.pSetLayouts = &resources.set;
    VkDescriptorSet set;
    check(vkAllocateDescriptorSets(device, &allocate, &set));
    VkDescriptorBufferInfo buffers[] = {{input.getBuffer(), 0, bytes}, {output.getBuffer(), 0, 24}};
    for (unsigned i = 0; i < 2; ++i) {
        VkWriteDescriptorSet write{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, i, 0, 1,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &buffers[i]
        };
        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }
    VkPipelineLayoutCreateInfo layout{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layout.setLayoutCount = 1;
    layout.pSetLayouts = &resources.set;
    check(vkCreatePipelineLayout(device, &layout, nullptr, &resources.layout));
    std::ifstream stream(shaderPath, std::ios::binary | std::ios::ate);
    std::vector<uint32_t> code(size_t(stream.tellg()) / 4);
    stream.seekg(0);
    stream.read(reinterpret_cast<char *>(code.data()), code.size() * 4);
    VkShaderModuleCreateInfo module{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    module.codeSize = code.size() * 4;
    module.pCode = code.data();
    check(vkCreateShaderModule(device, &module, nullptr, &resources.module));
    VkComputePipelineCreateInfo pipeline{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    pipeline.stage = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
        VK_SHADER_STAGE_COMPUTE_BIT, resources.module, "main"
    };
    pipeline.layout = resources.layout;
    check(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline, nullptr, &resources.pipeline));
    VKCommandPool commands(gpu);
    commands.AddCommandBuffer();
    auto *record = commands.getCommandBuffer(0);
    record->BindBuffer();
    auto cmd = record->getCommandBuffer();
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, resources.pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, resources.layout, 0, 1, &set, 0, nullptr);
    vkCmdDispatch(cmd, 1, 1, 1);
    VkMemoryBarrier barrier{
        VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT
    };
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &barrier, 0,
                         nullptr, 0, nullptr);
    record->UnbindBuffer();
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    auto queue = gpu->getGraphicsQueue().getQueue();
    check(vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE));
    check(vkQueueWaitIdle(queue));
    check(vkMapMemory(device, output.getMemory(), 0, 24, 0, &mapped));
    std::array<uint32_t, 6> results;
    std::memcpy(results.data(), mapped, 24);
    vkUnmapMemory(device, output.getMemory());
    for (unsigned i = 0; i < results.size(); ++i)
        if (results[i] != 1) throw std::runtime_error("Cascade GPU traversal check failed: " + std::to_string(i));
    std::fprintf(
        stderr, "CASCADE TRAVERSAL PASS: far hits, fine reentry, outer sky, finite rays and overlap conservation\n");
}