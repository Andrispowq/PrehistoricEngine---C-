#include "engine/prehistoric/core/util/Includes.hpp"
#include "RendererComponent.h"
#include "engine/prehistoric/common/rendering/pipeline/Pipeline.h"
#include "engine/prehistoric/engines/RenderingEngine.h"
#include "engine/prehistoric/core/gameObject/GameObject.h"
#include "engine/prehistoric/core/model/material/Material.h"

#include "engine/prehistoric/resources/AssembledAssetManager.h"

RendererComponent::RendererComponent(size_t pipelineID, size_t materialID, Window* window, AssembledAssetManager* manager)
	: RenderableComponent(pipelineID, window, manager)
{
	materialIndex = materialID;
	manager->addReference<Material>(materialID);

	if (FrameworkConfig::api == Vulkan)
	{
		static_cast<VKShader*>(manager->getResourceByID<Pipeline>(pipelineID)->getShader())->RegisterInstance();
	}
}

RendererComponent::RendererComponent(Window* window, AssembledAssetManager* manager)
	: RenderableComponent(window, manager)
{
	materialIndex = -1;
}

RendererComponent::~RendererComponent()
{
	manager->removeReference<Material>(materialIndex);
	materialIndex = -1;
}

void RendererComponent::PreRender(Renderer* renderer)
{
	renderer->AddModel(this);
}

void RendererComponent::Render(Renderer* renderer) const
{
	Pipeline* pipeline = getPipeline();
	
	pipeline->BindPipeline(renderer->getDrawCommandBuffer());
	pipeline->getShader()->UpdateShaderUniforms(renderer->getCamera(), renderer->getLights());
	pipeline->getShader()->UpdateSharedUniforms(parent);
	pipeline->getShader()->UpdateObjectUniforms(parent);

	pipeline->RenderPipeline();
	pipeline->UnbindPipeline();
}

void RendererComponent::BatchRender(uint32_t instance_index) const
{
	Pipeline* pipeline = getPipeline();

	pipeline->getShader()->UpdateObjectUniforms(parent, instance_index);
	pipeline->RenderPipeline();
}

Material* RendererComponent::getMaterial() const
{
	return manager->getResourceByID<Material>(materialIndex);
}
