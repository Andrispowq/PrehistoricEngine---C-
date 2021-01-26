#ifndef VK_PIPELINE_H
#define VK_PIPELINE_H

#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/surface/VKSurface.h"
#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"

namespace Prehistoric
{
	class VKPipeline : public Pipeline
	{
	public:
		VKPipeline(Window* window, AssetManager* manager, ShaderHandle shader);
		virtual ~VKPipeline() {}

		virtual void BindPipeline(CommandBuffer* buffer) const override { shader->Bind(buffer); this->buffer = buffer; }
		virtual void RenderPipeline() const override {}
		virtual void UnbindPipeline() const override {}

		virtual void RecreatePipeline() {}
	protected:
		VKDevice* device;
		VKSurface* surface;

		Window* window;
		VKSwapchain* swapchain;
	};
};

#endif