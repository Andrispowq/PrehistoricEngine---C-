#ifndef VK_RENDERER_H
#define VK_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include "platform/vulkan/rendering/command/VKCommandPool.h"
#include "platform/vulkan/rendering/renderpass/VKRenderpass.h"

namespace Prehistoric
{
	class VKRenderer : public Renderer
	{
	public:
		VKRenderer(Window* window, Camera* camera, AssembledAssetManager* manager);
		virtual ~VKRenderer() {}

		virtual void PrepareRendering();
		virtual void EndRendering();

		virtual void Render() override;

		virtual CommandBuffer* getDrawCommandBuffer() const { return (CommandBuffer*)commandPool->getCommandBuffer(window->getSwapchain()->getAquiredImageIndex()); }
	private:
		std::unique_ptr<VKCommandPool> commandPool;
		std::unique_ptr<VKRenderpass> renderpass;

		std::vector<std::unique_ptr<VKFramebuffer>> primaryFramebuffers;
	};
};

#endif