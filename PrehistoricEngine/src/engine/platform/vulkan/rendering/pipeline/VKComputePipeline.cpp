#include "Includes.hpp"
#include "VKComputePipeline.h"

namespace Prehistoric
{
	VKComputePipeline::VKComputePipeline(Window* window, AssetManager* manager, ShaderHandle shader)
		: VKPipeline(window, manager, shader)
	{
		CreatePipeline();
	}

	VKComputePipeline::~VKComputePipeline()
	{
		vkDestroyPipeline(device->getDevice(), computePipeline, nullptr);
	}

	void VKComputePipeline::BindPipeline(CommandBuffer* buffer) const
	{
		VKPipeline::BindPipeline(buffer);
		vkCmdBindPipeline(((VKCommandBuffer*)buffer)->getCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
	}

	void VKComputePipeline::RenderPipeline() const
	{
		VKPipeline::RenderPipeline();
		vkCmdDispatch(((VKCommandBuffer*)buffer)->getCommandBuffer(), invocationSize.x, invocationSize.y, invocationSize.z);
	}

	void VKComputePipeline::UnbindPipeline() const
	{
		VKPipeline::UnbindPipeline();
	}

	void VKComputePipeline::RecreatePipeline()
	{
		VKPipeline::RecreatePipeline();

		vkDestroyPipeline(device->getDevice(), computePipeline, nullptr);
		CreatePipeline();
	}

	uint64_t VKComputePipeline::GetHash()
	{
		if (hash)
			return hash;

		SetHashInternal(APIHashFlags::VK, PipelineTypeHashFlags::Compute, shader.handle, 0);
		return hash;
	}

	void VKComputePipeline::CreatePipeline()
	{
		this->renderpass = ((VKSwapchain*)window->getSwapchain())->getRenderpass();

		VKShader* shader = (VKShader*)getShader();

		//The actual creation
		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = shader->GetShaderStages()[0];
		pipelineInfo.layout = shader->GetPipelineLayout();
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateComputePipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Compute pipeline creation has failed!\n");
		}
	}
}