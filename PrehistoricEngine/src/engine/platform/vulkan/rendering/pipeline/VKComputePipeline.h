#ifndef VK_COMPUTE_PIPELINE_H
#define VK_COMPUTE_PIPELINE_H

#include "prehistoric/common/rendering/pipeline/ComputePipeline.h"
#include "VKPipeline.h"

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/rendering/renderpass/VKRenderpass.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"

#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	class VKComputePipeline : public VKPipeline, public ComputePipeline
	{
	public:
		VKComputePipeline(Window* window, AssetManager* manager, ShaderHandle shader);
		virtual ~VKComputePipeline();

		virtual void BindPipeline(CommandBuffer* buffer) const override;
		virtual void RenderPipeline() const override;
		virtual void UnbindPipeline() const override;

		virtual void RecreatePipeline();

		virtual uint64_t VKComputePipeline::GetHash() override;

		VkPipeline getComputePipeline() const { return computePipeline; }
	private:
		void CreatePipeline();
	private:
		VKRenderpass* renderpass;

		VkPipeline computePipeline;
	};
}

#endif