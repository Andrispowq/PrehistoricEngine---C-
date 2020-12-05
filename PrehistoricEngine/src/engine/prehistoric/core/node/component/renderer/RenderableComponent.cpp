#include "Includes.hpp"
#include "RenderableComponent.h"
#include "prehistoric/core/model/material/Material.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"
#include "prehistoric/common/rendering/pipeline/GraphicsPipeline.h"
#include "prehistoric/common/rendering/pipeline/ComputePipeline.h"

#include "platform/vulkan/rendering/shaders/VKShader.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	RenderableComponent::RenderableComponent(size_t pipelineID, Window* window, AssembledAssetManager* manager)
		: priority(RenderPriority::_3D)
	{
		this->window = window;
		this->manager = manager;

		pipelineIndex = pipelineID;
		manager->addReference<Pipeline>(pipelineID);
	}

	RenderableComponent::RenderableComponent(Window* window, AssembledAssetManager* manager)
		: priority(RenderPriority::_3D)
	{
		pipelineIndex = -1;

		this->window = window;
		this->manager = manager;
	}

	RenderableComponent::~RenderableComponent()
	{
		manager->removeReference<Pipeline>(pipelineIndex);
		pipelineIndex = -1;
	}

	Pipeline* RenderableComponent::getPipeline() const
	{
		return manager->getResourceByID<Pipeline>(pipelineIndex);
	}
};
