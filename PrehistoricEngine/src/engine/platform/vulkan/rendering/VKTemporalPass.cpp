#include "Includes.hpp"
#include "VKTemporalPass.h"
#include "VKRenderer.h"
#include "renderpass/VKRenderpass.h"
#include "pipeline/VKGraphicsPipeline.h"
#include "prehistoric/core/node/component/renderer/RenderableComponent.h"

namespace Prehistoric {
    static void TemporalCheck(VkResult result) {
        if (result != VK_SUCCESS)
            throw std::runtime_error("Vulkan temporal resource failure: " + std::to_string(result));
    }

    VKTemporalPass::VKTemporalPass(VKRenderpass &pass)
        : owner(pass), device(pass.device->getDevice()), extent(pass.swapchain->getSwapchainExtent()) {
        auto samples = owner.swapchain->getSceneSamples();
        VkImageFormatProperties supported{};
        auto supports = [&](VkFormat format) {
            return vkGetPhysicalDeviceImageFormatProperties(
                       owner.device->getPhysicalDevice(), format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, &supported) == VK_SUCCESS &&
                   (supported.sampleCounts & samples);
        };
        if (!supports(motionFormat)) {
            motionFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
            if (!supports(motionFormat))
                throw std::runtime_error("Temporal MSAA target format is unsupported");
        }

        VkDescriptorSetLayoutBinding bindings[7]{};
        for (unsigned i = 0; i < 7; ++i)
            bindings[i] = {
                i, i == 6 ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                VkShaderStageFlags(i == 6
                                       ? VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
                                       : VK_SHADER_STAGE_FRAGMENT_BIT),
                nullptr
            };
        VkDescriptorSetLayoutCreateInfo setInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        setInfo.bindingCount = 7;
        setInfo.pBindings = bindings;
        TemporalCheck(vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &setLayout));
        VkPushConstantRange range{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4};
        VkPipelineLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &setLayout;
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &range;
        TemporalCheck(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout));
        range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 128};
        TemporalCheck(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &geometryLayout));
        CreateVelocityPipeline();
        motionPass = owner.CreateOutputPass(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        for (unsigned i = 0; i < 8; ++i) {
            VkExtent2D size = i >= 6 ? VkExtent2D{(extent.width + 15) / 16, (extent.height + 15) / 16} : extent;
            images[i] = std::make_unique<VKImage>(owner.device, size.width, size.height, 1,
                                                  i == 0
                                                      ? motionFormat
                                                      : i <= 2
                                                            ? VK_FORMAT_R32G32B32A32_SFLOAT
                                                            : VK_FORMAT_R16G16B16A16_SFLOAT,
                                                  VK_IMAGE_TILING_OPTIMAL,
                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                  i == 0 ? samples : VK_SAMPLE_COUNT_1_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
            if (i != 0) {
                auto view = images[i]->getImageView();
                frames[i] = std::make_unique<VKFramebuffer>(device, i <= 2 ? motionPass : owner.bloomPass, size.width,
                                                            size.height, 1, &view);
            }
        }
        depth = std::make_unique<VKImage>(owner.device, extent.width, extent.height, 1, VK_FORMAT_D32_SFLOAT,
                                          VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, samples,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
        VkImageView views[] = {images[0]->getImageView(), depth->getImageView()};
        frames[0] = std::make_unique<VKFramebuffer>(device, velocityPass, extent.width, extent.height, 2, views);
        resolve =
                owner.CreateFullscreenPipeline("vulkan/temporal/fullscreen.vert.spv",
                                               samples == VK_SAMPLE_COUNT_1_BIT
                                                   ? "vulkan/temporal/resolve.frag.spv"
                                                   : "vulkan/temporal/resolve_ms.frag.spv",
                                               layout, motionPass);
        filter = owner.CreateFullscreenPipeline("vulkan/temporal/fullscreen.vert.spv",
                                                "vulkan/temporal/filter.frag.spv", layout, owner.bloomPass);
        slots.resize(owner.swapchain->getSwapchainImages().size());
        queried.resize(slots.size(), false);
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(owner.device->getPhysicalDevice(), &properties);
        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(owner.device->getPhysicalDevice(), &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(owner.device->getPhysicalDevice(), &queueCount, queues.data());
        const auto bits = queues[owner.device->getQueueFamilyIndices().graphicsFamily].timestampValidBits;
        if (bits) {
            timestampMs = properties.limits.timestampPeriod * 1e-6;
            timestampMask = UINT64_MAX >> (64 - bits);
            VkQueryPoolCreateInfo queryInfo{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
            queryInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queryInfo.queryCount = unsigned(slots.size()) * 8;
            TemporalCheck(vkCreateQueryPool(device, &queryInfo, nullptr, &timers));
        }
        unsigned count = unsigned(slots.size()) * 5;
        VkDescriptorPoolSize sizes[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, count * 6},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, count}
        };
        VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        poolInfo.maxSets = count;
        poolInfo.poolSizeCount = 2;
        poolInfo.pPoolSizes = sizes;
        TemporalCheck(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));
        for (auto &slot: slots) {
            slot.uniforms = std::make_unique<VKBuffer>(
                owner.device, sizeof(Renderer::TemporalParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            VkDescriptorSetLayout layouts[] = {setLayout, setLayout, setLayout, setLayout, setLayout};
            VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
            alloc.descriptorPool = pool;
            alloc.descriptorSetCount = 5;
            alloc.pSetLayouts = layouts;
            TemporalCheck(vkAllocateDescriptorSets(device, &alloc, slot.sets));
        }
    }

    VKTemporalPass::~VKTemporalPass() {
        vkDestroyQueryPool(device, timers, nullptr);
        for (auto pipeline: velocities)
            vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipeline(device, resolve, nullptr);
        vkDestroyPipeline(device, filter, nullptr);
        vkDestroyPipelineLayout(device, layout, nullptr);
        vkDestroyPipelineLayout(device, geometryLayout, nullptr);
        vkDestroyDescriptorPool(device, pool, nullptr);
        vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
        for (auto &frame: frames)
            frame.reset();
        vkDestroyRenderPass(device, velocityPass, nullptr);
        vkDestroyRenderPass(device, motionPass, nullptr);
    }

    void VKTemporalPass::CreateVelocityPipeline() {
        auto samples = owner.swapchain->getSceneSamples();
        VkAttachmentDescription attachments[2]{};
        attachments[0].format = motionFormat;
        attachments[1].format = VK_FORMAT_D32_SFLOAT;
        for (auto &attachment: attachments) {
            attachment.samples = samples;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colour{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                depthRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colour;
        subpass.pDepthStencilAttachment = &depthRef;
        VkSubpassDependency dependencies[] = {
            {
                VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, 0
            },
            {
                0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                0
            }
        };
        VkRenderPassCreateInfo pass{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        pass.attachmentCount = 2;
        pass.pAttachments = attachments;
        pass.subpassCount = 1;
        pass.pSubpasses = &subpass;
        pass.dependencyCount = 2;
        pass.pDependencies = dependencies;
        TemporalCheck(vkCreateRenderPass(device, &pass, nullptr, &velocityPass));
        VkShaderModule modules[2]{};
        VkPipelineShaderStageCreateInfo stages[2]{};
        const char *paths[] = {"vulkan/temporal/velocity.vert.spv", "vulkan/temporal/velocity.frag.spv"};
        for (unsigned i = 0; i < 2; ++i) {
            auto code = ResourceLoader::LoadShaderVK(paths[i]);
            VkShaderModuleCreateInfo module{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            module.codeSize = code.size();
            module.pCode = reinterpret_cast<const uint32_t *>(code.data());
            TemporalCheck(vkCreateShaderModule(device, &module, nullptr, &modules[i]));
            stages[i] = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                nullptr,
                0,
                i == 0 ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT,
                modules[i],
                "main",
                nullptr
            };
        }
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
        raster.lineWidth = 1;
        VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisample.rasterizationSamples = samples;
        VkPipelineDepthStencilStateCreateInfo test{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        test.depthTestEnable = test.depthWriteEnable = VK_TRUE;
        test.depthCompareOp = VK_COMPARE_OP_LESS;
        VkPipelineColorBlendAttachmentState colourWrite{};
        colourWrite.colorWriteMask = 15;
        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        blend.attachmentCount = 1;
        blend.pAttachments = &colourWrite;
        VkGraphicsPipelineCreateInfo create{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        create.stageCount = 2;
        create.pStages = stages;
        create.pVertexInputState = &input;
        create.pInputAssemblyState = &assembly;
        create.pViewportState = &viewport;
        create.pRasterizationState = &raster;
        create.pMultisampleState = &multisample;
        create.pDepthStencilState = &test;
        create.pColorBlendState = &blend;
        create.pDynamicState = &dynamic;
        create.layout = geometryLayout;
        create.renderPass = velocityPass;
        for (unsigned i = 0; i < 3; ++i) {
            raster.cullMode = i == 2 ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;
            raster.frontFace = i == 1 ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
            TemporalCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create, nullptr, &velocities[i]));
        }
        for (auto module: modules)
            vkDestroyShaderModule(device, module, nullptr);
    }

    VkImageView VKTemporalPass::Render(VKRenderer &renderer) {
        renderer.temporalRendered = true;
        auto command = renderer.getDrawCommandBuffer()->getCommandBuffer();
        auto &history = renderer.temporalHistory;
        const unsigned slotIndex = owner.swapchain->getAquiredImageIndex();
        auto &slot = slots[slotIndex];
        const unsigned queryBase = slotIndex * 8;
        if (timers) {
            if (queried[slotIndex]) {
                uint64_t times[8]{};
                if (vkGetQueryPoolResults(device, timers, queryBase, 8, sizeof(times), times, sizeof(uint64_t),
                                          VK_QUERY_RESULT_64_BIT) == VK_SUCCESS) {
                    std::array < double, 4 > values{};
                    for (unsigned i = 0; i < 4; ++i) {
                        values[i] = double((times[2 * i + 1] - times[2 * i]) & timestampMask) * timestampMs;
                    }
                    renderer.temporalTimings.Publish(slot.timingFrame, slot.timingMode, values);
                }
            }
            vkCmdResetQueryPool(command, timers, queryBase, 8);
            queried[slotIndex] = true;
            slot.timingFrame = renderer.submittedFrames + 1;
            slot.timingMode = unsigned(renderer.temporal.antialiasing) | (unsigned(renderer.temporal.motionBlur) << 1);
        }
        auto stamp = [&](unsigned i) {
            if (timers)
                vkCmdWriteTimestamp(command, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timers, queryBase + i);
        };
        renderer.temporalBytes = uint64_t(extent.width) * extent.height *
                                 ((motionFormat == VK_FORMAT_R32G32B32A32_SFLOAT ? 20 : 12) *
                                  unsigned(owner.swapchain->getSceneSamples()) +
                                  56) +
                                 uint64_t((extent.width + 15) / 16) * ((extent.height + 15) / 16) * 16;
        if (!initialized)
            history.valid = false;
        auto params = renderer.GetTemporalParameters();
        params.depthRow = {history.projection.m[10], history.projection.m[14], 0, 0};
        slot.uniforms->MapMemory();
        slot.uniforms->Store(&params);
        slot.uniforms->UnmapMemory();
        const unsigned write = unsigned(history.frame % 2), read = 1 - write;
        auto image = [&](unsigned index) { return images[index]->getImageView(); };
        auto scene = owner.swapchain->getHDRImage()->getImageView();
        auto source = scene;
        auto update = [&](unsigned index, VkImageView first, VkImageView tile) {
            VkDescriptorImageInfo inputs[] = {
                {index == 0 ? owner.depthSampler : owner.sampler, first, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                {owner.depthSampler, image(1 + write), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                {owner.sampler, image(3 + read), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                {owner.depthSampler, image(1 + read), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                {owner.depthSampler, tile, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                {
                    owner.depthSampler, owner.swapchain->getDepthImage()->getImageView(),
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                }
            };
            VkDescriptorBufferInfo uniform{slot.uniforms->getBuffer(), 0, sizeof(params)};
            VkWriteDescriptorSet writes[7]{};
            for (unsigned i = 0; i < 7; ++i) {
                writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[i].dstSet = slot.sets[index];
                writes[i].dstBinding = i;
                writes[i].descriptorCount = 1;
                writes[i].descriptorType =
                        i == 6 ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                if (i == 6)
                    writes[i].pBufferInfo = &uniform;
                else
                    writes[i].pImageInfo = &inputs[i];
            }
            vkUpdateDescriptorSets(device, 7, writes, 0, nullptr);
        };
        auto begin = [&](unsigned target) {
            VkClearValue clear[2]{};
            clear[1].depthStencil = {1, 0};
            VkRenderPassBeginInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            info.renderPass = target == 0 ? velocityPass : target <= 2 ? motionPass : owner.bloomPass;
            info.framebuffer = frames[target]->getFramebuffer();
            info.renderArea.extent = frames[target]->getExtent();
            info.clearValueCount = target == 0 ? 2 : 0;
            info.pClearValues = clear;
            vkCmdBeginRenderPass(command, &info, VK_SUBPASS_CONTENTS_INLINE);
            auto size = info.renderArea.extent;
            VkViewport viewport{0, 0, float(size.width), float(size.height), 0, 1};
            vkCmdSetViewport(command, 0, 1, &viewport);
            vkCmdSetScissor(command, 0, 1, &info.renderArea);
        };
        if (!initialized) {
            for (unsigned i = 1; i < 8; ++i) {
                begin(i);
                VkClearAttachment clear{};
                clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                VkClearRect rect{{{0, 0}, frames[i]->getExtent()}, 0, 1};
                vkCmdClearAttachments(command, 1, &clear, 1, &rect);
                vkCmdEndRenderPass(command);
            }
            initialized = true;
        }
        stamp(0);
        update(0, image(0), image(7));
        begin(0);
        vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, geometryLayout, 0, 1, &slot.sets[0], 0,
                                nullptr);
        VertexBuffer *vertices = nullptr;
        for (const auto *batches: {&renderer.getModels3D(), &renderer.getModelsTransparency()})
            Renderer::VisitBatches(
                *batches,
                [&](Pipeline *pipeline) {
                    auto *graphics = static_cast<VKGraphicsPipeline *>(pipeline);
                    vertices = graphics->getTopology() == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
                                   ? graphics->getVertexBuffer()
                                   : nullptr;
                    if (!vertices || vertices->getType() != VertexBufferType::MESH) {
                        vertices = nullptr;
                        return;
                    }
                    vertices->Bind(renderer.getDrawCommandBuffer());
                    const unsigned front =
                            !graphics->isBackfaceCulling() || graphics->getFrontFace() == FrontFace::DOUBLE_SIDED
                                ? 2
                                : graphics->getFrontFace() == FrontFace::COUNTER_CLOCKWISE
                                      ? 1
                                      : 0;
                    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, velocities[front]);
                },
                [](Pipeline *, Material *) {
                },
                [&](Pipeline *, RenderableComponent *object) {
                    if (!vertices)
                        return;
                    const auto &model = history.pendingModels.at(object->getTemporalIdentity());
                    const auto old = history.models.find(object->getTemporalIdentity());
                    bool valid = history.CanReproject(object->getTemporalIdentity(), model) &&
                                 object->getPriority() == RenderPriority::_3D;
                    const auto current = history.current * model;
                    const auto previous = valid ? history.previous * old->second : Matrix4f::Zero();
                    float transforms[32];
                    std::copy_n(current.m, 16, transforms);
                    std::copy_n(previous.m, 16, transforms + 16);
                    vkCmdPushConstants(command, geometryLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 128, transforms);
                    static_cast<VKMeshVertexBuffer *>(vertices)->DrawAll(renderer.getDrawCommandBuffer());
                },
                [](Pipeline *) {
                });
        vkCmdEndRenderPass(command);
        auto draw = [&](unsigned target, unsigned set, VkPipeline pipeline, int mode) {
            begin(target);
            vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &slot.sets[set], 0,
                                    nullptr);
            vkCmdPushConstants(command, layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4, &mode);
            vkCmdDraw(command, 3, 1, 0, 0);
            vkCmdEndRenderPass(command);
        };
        stamp(1);
        stamp(2);
        draw(1 + write, 0, resolve, 0);
        stamp(3);
        stamp(4);
        update(1, source, image(7));
        if (renderer.temporal.antialiasing || renderer.temporal.debug >= 3) {
            draw(3 + write, 1, filter, 0);
            source = image(3 + write);
        }
        stamp(5);
        stamp(6);
        if (renderer.temporal.debug >= 3) {
            draw(5, 1, filter, 4);
            source = image(5);
        } else if (renderer.temporal.motionBlur || renderer.temporal.debug == 1 || renderer.temporal.debug == 2) {
            update(2, source, image(7));
            draw(6, 2, filter, 2);
            update(3, source, image(6));
            draw(7, 3, filter, 3);
            update(4, source, image(7));
            draw(5, 4, filter, 1);
            source = image(5);
        }
        stamp(7);
        return source;
    }
} // namespace Prehistoric