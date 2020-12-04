#include "engine/prehistoric/core/util/Includes.hpp"
#include "RenderableComponent.h"
#include "engine/prehistoric/core/model/material/Material.h"
#include "engine/prehistoric/common/rendering/pipeline/Pipeline.h"
#include "engine/prehistoric/common/rendering/pipeline/GraphicsPipeline.h"
#include "engine/prehistoric/common/rendering/pipeline/ComputePipeline.h"

#include "engine/platform/vulkan/rendering/shaders/VKShader.h"

#include "engine/prehistoric/resources/AssembledAssetManager.h"

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
