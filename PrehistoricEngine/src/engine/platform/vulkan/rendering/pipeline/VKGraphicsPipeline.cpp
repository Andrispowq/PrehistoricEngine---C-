#include "Includes.hpp"
#include "VKGraphicsPipeline.h"
#include "platform/vulkan/rendering/shaders/pbr/VKPBRShader.h"

namespace Prehistoric {
    VKGraphicsPipeline::VKGraphicsPipeline(Window *window, AssetManager *manager, ShaderHandle shader,
                                           VertexBufferHandle vbo)
        : VKPipeline(window, manager, shader), GraphicsPipeline(manager, vbo) {
        CreatePipeline();
    }

    VKGraphicsPipeline::~VKGraphicsPipeline() {
        vkDestroyPipeline(device->getDevice(), graphicsPipeline, nullptr);
        vkDestroyPipeline(device->getDevice(), wireframePipeline, nullptr);
        vkDestroyPipeline(device->getDevice(), receiverPipeline, nullptr);
        receiverPipeline = VK_NULL_HANDLE;
    }

    void VKGraphicsPipeline::BindPipeline(CommandBuffer *buffer) const {
        BindPipeline(buffer, false);
    }

    void VKGraphicsPipeline::BindPipeline(CommandBuffer *buffer, bool wireframe, bool receivers) const {
        VKPipeline::BindPipeline(buffer);

        if (buffer) {
            vkCmdBindPipeline(((VKCommandBuffer *) buffer)->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                              receivers
                                  ? receiverPipeline
                                  : (wireframe && wireframePipeline ? wireframePipeline : graphicsPipeline));
            vbo->Bind(buffer);
        }
    }

    void VKGraphicsPipeline::RenderPipeline() const {
        VKPipeline::RenderPipeline();

        for (uint32_t i = 0; i < vbo->getSubmeshCount(); i++) {
            vbo->Draw(buffer, i);
        }
    }

    void VKGraphicsPipeline::UnbindPipeline() const {
        vbo->Unbind();

        VKPipeline::UnbindPipeline();
    }

    void VKGraphicsPipeline::RecreatePipeline() {
        VKPipeline::RecreatePipeline();

        vkDestroyPipeline(device->getDevice(), graphicsPipeline, nullptr);
        vkDestroyPipeline(device->getDevice(), wireframePipeline, nullptr);
        vkDestroyPipeline(device->getDevice(), receiverPipeline, nullptr);
        receiverPipeline = VK_NULL_HANDLE;
        CreatePipeline();
    }

    uint64_t VKGraphicsPipeline::GetHash() {
        if (hash)
            return hash;

        SetHashInternal(APIHashFlags::VK, PipelineTypeHashFlags::Graphics, shader.handle, vbo.handle);
        return hash;
    }

    void VKGraphicsPipeline::CreatePipeline() {
        this->renderpass = ((VKSwapchain *) window->getSwapchain())->getRenderpass();

        VKMeshVertexBuffer *vbo = (VKMeshVertexBuffer *) getVertexBuffer();
        VKShader *shader = (VKShader *) getShader();

        //The actual creation
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        VkVertexInputBindingDescription desc = vbo->getBindingDescription();
        vertexInputInfo.pVertexBindingDescriptions = &desc;

        auto description = vbo->getAttributeDescriptions();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(description.size());
        vertexInputInfo.pVertexAttributeDescriptions = description.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPipelineTessellationStateCreateInfo tessellation{};
        tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellation.patchControlPoints = 16;
        for (uint32_t i = 0; i < shader->getModuleCount(); ++i) {
            if (shader->GetShaderStages()[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
                // Terrain control points use only the position from the mesh layout.
                vertexInputInfo.vertexAttributeDescriptionCount = 1;
                inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            }
        }
        topology = inputAssembly.topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkDynamicState states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamic.dynamicStateCount = 2;
        dynamic.pDynamicStates = states;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = backfaceCulling && vbo->getFrontFace() != FrontFace::DOUBLE_SIDED
                                  ? VK_CULL_MODE_BACK_BIT
                                  : VK_CULL_MODE_NONE;
        rasterizer.frontFace = vbo->getFrontFace() == FrontFace::COUNTER_CLOCKWISE
                                   ? VK_FRONT_FACE_COUNTER_CLOCKWISE
                                   : VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_TRUE;
        //This means that if the sampleCount is 1, 0 or -1, we disable multisampling, in all other cases, we use the max available sample count for MSAA
        multisampling.rasterizationSamples = static_cast<VKSwapchain *>(window->getSwapchain())->getSceneSamples();
        multisampling.minSampleShading = 0.2f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        VkPipelineColorBlendAttachmentState attachments[] = {colorBlendAttachment, colorBlendAttachment};
        colorBlending.attachmentCount = 2;
        colorBlending.pAttachments = attachments;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shader->getModuleCount();
        pipelineInfo.pStages = shader->GetShaderStages();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pTessellationState = inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
                                              ? &tessellation
                                              : nullptr;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamic;
        pipelineInfo.layout = shader->GetPipelineLayout();
        pipelineInfo.renderPass = renderpass->getRenderPass();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)
            != VK_SUCCESS) {
            PR_LOG_RUNTIME_ERROR("Graphics pipeline creation has failed!\n");
        }

        if (dynamic_cast<VKPBRShader *>(shader) && topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
            auto code = ResourceLoader::LoadShaderVK("vulkan/voxel/receiver.spv");
            VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            moduleInfo.codeSize = code.size();
            moduleInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
            VkShaderModule module{};
            if (vkCreateShaderModule(device->getDevice(), &moduleInfo, nullptr, &module) != VK_SUCCESS)
                throw std::runtime_error("Could not create the VXGI receiver shader");
            auto stages = std::vector<VkPipelineShaderStageCreateInfo>(pipelineInfo.pStages,
                                                                       pipelineInfo.pStages + pipelineInfo.stageCount);
            for (auto &stage: stages)
                if (stage.stage == VK_SHADER_STAGE_FRAGMENT_BIT) stage.module = module;
            pipelineInfo.pStages = stages.data();
            auto result = vkCreateGraphicsPipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                                    &receiverPipeline);
            vkDestroyShaderModule(device->getDevice(), module, nullptr);
            if (result != VK_SUCCESS) throw std::runtime_error("Could not create the VXGI receiver pipeline");
            pipelineInfo.pStages = shader->GetShaderStages();
        }

        VkPhysicalDeviceFeatures features{};
        vkGetPhysicalDeviceFeatures(device->getPhysicalDevice(), &features);
        if (features.fillModeNonSolid) {
            rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
            if (vkCreateGraphicsPipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                          &wireframePipeline) != VK_SUCCESS)
                PR_LOG_RUNTIME_ERROR("Wireframe pipeline creation failed!\n");
        }
    }
};