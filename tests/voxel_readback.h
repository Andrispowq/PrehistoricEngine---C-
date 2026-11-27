#pragma once
#include "tests/temporal_render_driver.h"
#include "platform/vulkan/rendering/command/VKCommandPool.h"

// Diagnostic readback at the smoke fixture's 64-cubed resolution. Compare geometry
// separately from light so an occupancy-only pass cannot hide stale radiance.
inline std::vector<uint32_t> ReadVoxelFixture(Prehistoric::Renderer &renderer) {
    using namespace Prehistoric;
    constexpr uint32_t count = 64 * 64 * 64, pyramid = (8 * count - 1) / 7;
    std::vector<uint32_t> words(5 * count);
    if (auto *gl = dynamic_cast<GLRenderer *>(&renderer)) {
        glFinish();
        auto *pass = gl->getVoxelPass();
        const GLuint textures[] = {pass->GetOccluderTexture(), pass->GetSurfaceTexture(), pass->GetEmissionTexture()};
        for (unsigned i = 0; i < 3; ++i)
            glGetTextureImage(textures[i], 0, GL_RED_INTEGER, GL_UNSIGNED_INT, count * 4, words.data() + i * count);
        glGetTextureImage(pass->GetRadianceTexture(), 0, GL_RGBA, GL_HALF_FLOAT, count * 8, words.data() + 3 * count);
        return words;
    }
    auto *vk = static_cast<VKRenderer *>(&renderer);
    auto *gpu = static_cast<VKDevice *>(renderer.getWindow()->getContext()->getDevices());
    vkDeviceWaitIdle(gpu->getDevice());
    VKBuffer output(gpu, words.size() * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VKCommandPool commands(gpu);
    commands.AddCommandBuffer();
    auto *record = commands.getCommandBuffer(0);
    record->BindBuffer();
    auto cmd = record->getCommandBuffer();
    VkMemoryBarrier barrier{
        VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT
    };
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &barrier, 0,
                         nullptr, 0, nullptr);
    const VkDeviceSize offsets[] = {0, pyramid, pyramid + count, (pyramid + 2 * count + 1) & ~1u};
    for (unsigned i = 0; i < 4; ++i) {
        VkBufferCopy copy{offsets[i] * 4, VkDeviceSize(i * count) * 4, VkDeviceSize(count) * (i == 3 ? 8u : 4u)};
        vkCmdCopyBuffer(cmd, vk->GetVoxelBuffer(), output.getBuffer(), 1, &copy);
    }
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &barrier, 0, nullptr, 0,
                         nullptr);
    record->UnbindBuffer();
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    auto queue = gpu->getGraphicsQueue().getQueue();
    if (vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE) != VK_SUCCESS || vkQueueWaitIdle(queue) != VK_SUCCESS)
        throw std::runtime_error("Voxel readback submission failed");
    void *mapped = nullptr;
    if (vkMapMemory(gpu->getDevice(), output.getMemory(), 0, output.getSize(), 0, &mapped) != VK_SUCCESS)
        throw std::runtime_error("Voxel readback mapping failed");
    std::memcpy(words.data(), mapped, output.getSize());
    vkUnmapMemory(gpu->getDevice(), output.getMemory());
    return words;
}