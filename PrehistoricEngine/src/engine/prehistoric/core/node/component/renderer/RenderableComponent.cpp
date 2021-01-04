#include "Includes.hpp"
#include "RenderableComponent.h"
#include "prehistoric/core/model/material/Material.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"
#include "prehistoric/common/rendering/pipeline/GraphicsPipeline.h"
#include "prehistoric/common/rendering/pipeline/ComputePipeline.h"

#include "platform/vulkan/rendering/shaders/VKShader.h"

#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	RenderableComponent::RenderableComponent(PipelineHandle pipeline, Window* window, ResourceStorage* resourceStorage)
		: priority(RenderPriority::_3D)
	{
		type = ComponentType::RenderableComponent;

		this->window = window;
		this->resourceStorage = resourceStorage;

		this->pipeline = pipeline;
		resourceStorage->addReference<Pipeline>(pipeline.handle);
	}

	RenderableComponent::RenderableComponent(Window* window, ResourceStorage* resourceStorage)
		: priority(RenderPriority::_3D)
	{
		type = ComponentType::RendererComponent;

		this->window = window;
		this->resourceStorage = resourceStorage;
	}

	RenderableComponent::~RenderableComponent()
	{
		resourceStorage->removeReference<Pipeline>(pipeline.handle);
	}
};
