#ifndef VK_GRAPHICS_PIPELINE_H
#define VK_GRAPHICS_PIPELINE_H

#include "engine/prehistoric/common/rendering/pipeline/GraphicsPipeline.h"
#include "VKPipeline.h"

#include <vulkan/vulkan.h>

#include "engine/platform/vulkan/framework/device/VKDevice.h"
#include "engine/platform/vulkan/rendering/renderpass/VKRenderpass.h"
#include "engine/platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "engine/platform/vulkan/rendering/shaders/VkShader.h"

#include "engine/prehistoric/core/util/math/Math.h"

class VKGraphicsPipeline : public VKPipeline, public GraphicsPipeline
{
public:
	VKGraphicsPipeline(Window* window, AssetManager* manager, size_t shaderID, size_t vboID);
	virtual ~VKGraphicsPipeline();

	virtual void BindPipeline(CommandBuffer* buffer) const override;
	virtual void RenderPipeline() const override;
	virtual void UnbindPipeline() const override;

	virtual void RecreatePipeline();

	VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }
private:
	void CreatePipeline();
private:
	VKRenderpass* renderpass;

	VkPipeline graphicsPipeline;
};

#endif