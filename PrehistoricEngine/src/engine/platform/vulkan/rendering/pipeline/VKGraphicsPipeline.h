#ifndef VK_GRAPHICS_PIPELINE_H
#define VK_GRAPHICS_PIPELINE_H

#include "prehistoric/common/rendering/pipeline/GraphicsPipeline.h"
#include "VKPipeline.h"

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/rendering/renderpass/VKRenderpass.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"

#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	class VKGraphicsPipeline : public VKPipeline, public GraphicsPipeline
	{
	public:
		VKGraphicsPipeline(Window* window, AssetManager* manager, ShaderHandle shader, VertexBufferHandle vbo);
		virtual ~VKGraphicsPipeline();

		virtual void BindPipeline(CommandBuffer* buffer) const override;
		virtual void RenderPipeline() const override;
		virtual void UnbindPipeline() const override;

		virtual void RecreatePipeline();

		virtual uint64_t VKGraphicsPipeline::GetHash() override;

		VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }
	private:
		void CreatePipeline();
	private:
		VKRenderpass* renderpass;

		VkPipeline graphicsPipeline;
	};
};

#endif