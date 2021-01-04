#include "Includes.hpp"
#include "RendererComponent.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"
#include "prehistoric/core/engines/RenderingEngine.h"
#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/core/model/material/Material.h"

#include "prehistoric/core/resources/ResourceStorage.h"

#include "platform/vulkan/rendering/shaders/VKShader.h"

namespace Prehistoric
{
	RendererComponent::RendererComponent(PipelineHandle pipeline, MaterialHandle material, Window* window, ResourceStorage* resourceStorage)
		: RenderableComponent(pipeline, window, resourceStorage)
	{
		type = ComponentType::RendererComponent;

		this->material = material;
		resourceStorage->addReference<Material>(material.handle);

		if (FrameworkConfig::api == Vulkan)
		{
			static_cast<VKShader*>(pipeline->getShader())->RegisterInstance();
		}
	}

	RendererComponent::RendererComponent(Window* window, ResourceStorage* resourceStorage)
		: RenderableComponent(window, resourceStorage)
	{
		type = ComponentType::RendererComponent;
	}

	RendererComponent::~RendererComponent()
	{
		resourceStorage->removeReference<Material>(material.handle);
	}

	void RendererComponent::PreRender(Renderer* renderer)
	{
		renderer->AddModel(this);
	}

	void RendererComponent::Render(Renderer* renderer) const
	{
		pipeline->BindPipeline(renderer->getDrawCommandBuffer());
		pipeline->getShader()->UpdateShaderUniforms(renderer->getCamera(), renderer->getLights());
		pipeline->getShader()->UpdateSharedUniforms(parent);
		pipeline->getShader()->UpdateObjectUniforms(parent);

		pipeline->RenderPipeline();
		pipeline->UnbindPipeline();
	}

	void RendererComponent::BatchRender(uint32_t instance_index) const
	{
		pipeline->getShader()->UpdateObjectUniforms(parent, instance_index);
		pipeline->RenderPipeline();
	}
};
