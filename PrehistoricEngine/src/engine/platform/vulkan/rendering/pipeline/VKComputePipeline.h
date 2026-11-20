#ifndef VK_COMPUTE_PIPELINE_H
#define VK_COMPUTE_PIPELINE_H

#include "prehistoric/common/rendering/pipeline/ComputePipeline.h"
#include "VkPipeline.h"

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"

#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	class VKComputePipeline : public VKPipeline, public ComputePipeline
	{
	public:
		VKComputePipeline(Window* window, AssetManager* manager, ShaderHandle shader);
		//The device, shader module and layout must outlive this pipeline.
		VKComputePipeline(VkDevice device, const VkPipelineShaderStageCreateInfo& stage, VkPipelineLayout layout);
		virtual ~VKComputePipeline();

		VKComputePipeline(const VKComputePipeline&) = delete;
		VKComputePipeline& operator=(const VKComputePipeline&) = delete;

		void BindPipeline(VkCommandBuffer buffer) const;

		virtual void BindPipeline(CommandBuffer* buffer) const override;
		virtual void RenderPipeline() const override;
		virtual void UnbindPipeline() const override;

		virtual void RecreatePipeline() override;

		virtual uint64_t GetHash() override;

		VkPipeline getComputePipeline() const { return computePipeline; }
	private:
		void CreatePipeline();
	private:
		VkDevice logicalDevice;
		VkPipelineShaderStageCreateInfo stage;
		VkPipelineLayout layout;
		mutable VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

		VkPipeline computePipeline = VK_NULL_HANDLE;
	};
}

#endif