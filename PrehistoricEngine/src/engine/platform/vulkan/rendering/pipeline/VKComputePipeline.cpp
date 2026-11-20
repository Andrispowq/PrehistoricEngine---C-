#include "Includes.hpp"
#include "VKComputePipeline.h"

#include <stdexcept>

namespace Prehistoric
{
	VKComputePipeline::VKComputePipeline(Window* window, AssetManager* manager, ShaderHandle shader)
		: VKPipeline(window, manager, shader)
	{
		VKShader* computeShader = dynamic_cast<VKShader*>(getShader());
		if (!computeShader || computeShader->getModuleCount() != 1)
			throw std::runtime_error("A compute pipeline needs exactly one compute shader!");

		logicalDevice = device->getDevice();
		stage = computeShader->GetShaderStages()[0];
		layout = computeShader->GetPipelineLayout();
		CreatePipeline();
	}

	VKComputePipeline::VKComputePipeline(VkDevice device, const VkPipelineShaderStageCreateInfo& stage, VkPipelineLayout layout)
		: logicalDevice(device), stage(stage), layout(layout)
	{
		CreatePipeline();
	}

	VKComputePipeline::~VKComputePipeline()
	{
		vkDestroyPipeline(logicalDevice, computePipeline, nullptr);
	}

	void VKComputePipeline::BindPipeline(CommandBuffer* buffer) const
	{
		if (!buffer)
			throw std::runtime_error("A compute pipeline needs a command buffer!");

		if (getShader())
			VKPipeline::BindPipeline(buffer);
		else
			this->buffer = buffer;
		BindPipeline(((VKCommandBuffer*)buffer)->getCommandBuffer());
	}

	void VKComputePipeline::BindPipeline(VkCommandBuffer buffer) const
	{
		if (buffer == VK_NULL_HANDLE)
			throw std::runtime_error("A compute pipeline needs a command buffer!");

		commandBuffer = buffer;
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
	}

	void VKComputePipeline::RenderPipeline() const
	{
		if (commandBuffer == VK_NULL_HANDLE)
			throw std::runtime_error("Bind the compute pipeline before dispatching it!");

		//Vulkan resources are bound through shader descriptor sets.
		if (!textureBindingTable.empty() || !ssboBindingTable.empty())
			throw std::runtime_error("Use Vulkan descriptor sets for compute resources!");

		vkCmdDispatch(commandBuffer, invocationSize.x, invocationSize.y, invocationSize.z);
	}

	void VKComputePipeline::UnbindPipeline() const
	{
		commandBuffer = VK_NULL_HANDLE;
		VKPipeline::UnbindPipeline();
	}

	void VKComputePipeline::RecreatePipeline()
	{
		//The caller must finish any submissions using the old pipeline first.
		VkPipeline previous = computePipeline;
		CreatePipeline();
		vkDestroyPipeline(logicalDevice, previous, nullptr);
		commandBuffer = VK_NULL_HANDLE;
	}

	uint64_t VKComputePipeline::GetHash()
	{
		if (!getShader())
			throw std::runtime_error("Native compute pipelines do not have an asset handle!");

		if (hash)
			return hash;

		SetHashInternal(APIHashFlags::VK, PipelineTypeHashFlags::Compute, shader.handle, 0);
		return hash;
	}

	void VKComputePipeline::CreatePipeline()
	{
		if (stage.stage != VK_SHADER_STAGE_COMPUTE_BIT || stage.module == VK_NULL_HANDLE || layout == VK_NULL_HANDLE)
			throw std::runtime_error("Invalid compute shader or pipeline layout!");

		//The actual creation
		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = stage;
		pipelineInfo.layout = layout;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		VkPipeline pipeline = VK_NULL_HANDLE;
		VkResult result = vkCreateComputePipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
		if (result != VK_SUCCESS)
		{
			vkDestroyPipeline(logicalDevice, pipeline, nullptr);
			throw std::runtime_error("Compute pipeline creation has failed: " + std::to_string(result));
		}

		computePipeline = pipeline;
	}
}
