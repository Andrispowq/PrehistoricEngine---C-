#include "Includes.hpp"
#include "VKGraphicsPipeline.h"

namespace Prehistoric
{
	VKGraphicsPipeline::VKGraphicsPipeline(Window* window, AssetManager* manager, ShaderHandle shader, VertexBufferHandle vbo)
		: VKPipeline(window, manager, shader), GraphicsPipeline(manager, vbo)
	{
		CreatePipeline();
	}

	VKGraphicsPipeline::~VKGraphicsPipeline()
	{
		vkDestroyPipeline(device->getDevice(), graphicsPipeline, nullptr);
	}

	void VKGraphicsPipeline::BindPipeline(CommandBuffer* buffer) const
	{
		VKPipeline::BindPipeline(buffer);

		vkCmdBindPipeline(((VKCommandBuffer*)buffer)->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		vbo->Bind(buffer);
	}

	void VKGraphicsPipeline::RenderPipeline() const
	{
		VKPipeline::RenderPipeline();

		vbo->Draw(buffer);
	}

	void VKGraphicsPipeline::UnbindPipeline() const
	{
		vbo->Unbind();

		VKPipeline::UnbindPipeline();
	}

	void VKGraphicsPipeline::RecreatePipeline()
	{
		VKPipeline::RecreatePipeline();

		vkDestroyPipeline(device->getDevice(), graphicsPipeline, nullptr);
		CreatePipeline();
	}

	void VKGraphicsPipeline::CreatePipeline()
	{
		this->renderpass = ((VKSwapchain*)window->getSwapchain())->getRenderpass();

		VKMeshVertexBuffer* vbo = (VKMeshVertexBuffer*)getVertexBuffer();
		VKShader* shader = (VKShader*)getShader();

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
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = Pipeline::viewportStart.x;
		viewport.y = Pipeline::viewportStart.y;
		viewport.width = Pipeline::viewportStart.x + Pipeline::viewportSize.x;
		viewport.height = Pipeline::viewportStart.y + Pipeline::viewportSize.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { (int32_t)Pipeline::scissorStart.x, (int32_t)Pipeline::scissorStart.y };
		scissor.extent = { Pipeline::scissorStart.x + Pipeline::scissorSize.x, Pipeline::scissorStart.y + Pipeline::scissorSize.y };

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;// wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = backfaceCulling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
		rasterizer.frontFace = vbo->getFrontFace() == FrontFace::COUNTER_CLOCKWISE ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_TRUE;
		//This means that if the sampleCount is 1, 0 or -1, we disable multisampling, in all other cases, we use the max available sample count for MSAA
		multisampling.rasterizationSamples = /*samples < 2 ? VK_SAMPLE_COUNT_1_BIT :*/ device->getSampleCount();
		multisampling.minSampleShading = 0.2f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
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
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = shader->GetPipelineLayout();
		pipelineInfo.renderPass = renderpass->getRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Graphics pipeline creation has failed!\n");
		}
	}
};
