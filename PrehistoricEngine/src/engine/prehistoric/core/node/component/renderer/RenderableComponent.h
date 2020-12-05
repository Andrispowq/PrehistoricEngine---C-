#ifndef RENDERABLE_COMPONENT_H
#define RENDERABLE_COMPONENT_H

#include "prehistoric/core/node/component/Component.h"

#include "prehistoric/common/rendering/pipeline/Pipeline.h"
#include "prehistoric/common/framework/Window.h"

#include "Includes.hpp"

namespace Prehistoric
{
	class AssembledAssetManager;

	enum class RenderPriority
	{
		_3D = 0,
		_TRANSPARENCY = 1,
		_2D = 2,
		_POST_PROCESSING = 3
	};

	class RenderableComponent : public Component
	{
	public:
		RenderableComponent(size_t pipelineID, Window* window, AssembledAssetManager* manager);
		RenderableComponent(Window* window, AssembledAssetManager* manager);
		virtual ~RenderableComponent();

		virtual void Render(Renderer* renderer) const = 0;
		virtual void BatchRender(uint32_t instance_index = 0) const = 0;

		inline size_t getPipelineIndex() const { return pipelineIndex; }
		Pipeline* getPipeline() const;

		inline RenderPriority getPriority() const { return priority; }
		inline void setPriority(RenderPriority priority) { this->priority = priority; }

		RenderableComponent(const RenderableComponent&) = delete;
		RenderableComponent(const RenderableComponent&&) = delete;
		RenderableComponent& operator=(RenderableComponent) = delete;
	protected:
		AssembledAssetManager* manager;

		Window* window;

		size_t pipelineIndex;
		RenderPriority priority;
	};
};

#endif