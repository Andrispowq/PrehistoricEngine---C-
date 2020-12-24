#include "Includes.hpp"
#include "RendererComponent.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"
#include "prehistoric/core/engines/RenderingEngine.h"
#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/core/model/material/Material.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	RendererComponent::RendererComponent(size_t pipelineID, size_t materialID, Window* window, AssembledAssetManager* manager)
		: RenderableComponent(pipelineID, window, manager)
	{
		type = ComponentType::RendererComponent;

		materialIndex = materialID;
		manager->addReference<Material>(materialID);
		material = manager->getResourceByID<Material>(materialIndex);

		if (FrameworkConfig::api == Vulkan)
		{
			static_cast<VKShader*>(manager->getResourceByID<Pipeline>(pipelineID)->getShader())->RegisterInstance();
		}
	}

	RendererComponent::RendererComponent(Window* window, AssembledAssetManager* manager)
		: RenderableComponent(window, manager)
	{
		type = ComponentType::RendererComponent;

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
};
