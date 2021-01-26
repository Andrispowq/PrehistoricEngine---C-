#include "Includes.hpp"
#include "RendererComponent.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"
#include "prehistoric/core/engines/RenderingEngine.h"
#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/core/model/material/Material.h"

#include "platform/vulkan/rendering/shaders/VKShader.h"

namespace Prehistoric
{
	size_t RendererComponent::nextInstanceID = 0;

	RendererComponent::RendererComponent(Window* window, AssembledAssetManager* manager, PipelineHandle pipeline, MaterialHandle material)
		: RenderableComponent(window, manager, pipeline), material(material)
	{
		type = ComponentType::RendererComponent;

		manager->addReference<Material>(material.handle);

		if (FrameworkConfig::api == Vulkan)
		{
			static_cast<VKShader*>(pipeline->getShader())->RegisterInstance();
			instanceID = nextInstanceID++;
		}
	}

	RendererComponent::RendererComponent(Window* window, AssembledAssetManager* manager)
		: RenderableComponent(window, manager)
	{
		type = ComponentType::RendererComponent;
	}

	RendererComponent::~RendererComponent()
	{
		if(material.pointer != nullptr)
			manager->removeReference<Material>(material.handle);
	}

	void RendererComponent::PreRender(Renderer* renderer)
	{
		if(pipeline.pointer != nullptr)
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
		pipeline->getShader()->UpdateObjectUniforms(parent, (uint32_t)instanceID);
		pipeline->RenderPipeline();
	}
};
