#ifndef VK_PIPELINE_H
#define VK_PIPELINE_H

#include "engine/prehistoric/common/rendering/pipeline/Pipeline.h"

#include <vulkan/vulkan.h>

#include "engine/platform/vulkan/framework/surface/VKSurface.h"
#include "engine/platform/vulkan/framework/device/VKDevice.h"
#include "engine/platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "engine/platform/vulkan/buffer/VKMeshVertexBuffer.h"

class VKPipeline : public Pipeline
{
public:
	VKPipeline(Window* window, AssetManager* manager, size_t shaderID);
	virtual ~VKPipeline() {}

	virtual void BindPipeline(CommandBuffer* buffer) const override { getShader()->Bind(buffer); this->buffer = buffer; }
	virtual void RenderPipeline() const override {}
	virtual void UnbindPipeline() const override {}

	virtual void RecreatePipeline() {}
protected:
	VKPhysicalDevice* physicalDevice;
	VKDevice* device;
	VKSurface* surface;

	Window* window;
	VKSwapchain* swapchain;

	mutable CommandBuffer* buffer; //The currently used commandbuffer
};

#endif