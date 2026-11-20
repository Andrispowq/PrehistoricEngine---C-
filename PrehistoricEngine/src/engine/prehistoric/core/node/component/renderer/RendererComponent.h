#ifndef RENDERER_COMPONENT_H
#define RENDERER_COMPONENT_H

#include "Includes.hpp"

#include "RenderableComponent.h"

namespace Prehistoric
{
	class Material;

	class RendererComponent : public RenderableComponent
	{
	public:
		RendererComponent(Window* window, AssembledAssetManager* manager, PipelineHandle pipeline, MaterialHandle material);
		RendererComponent(Window* window, AssembledAssetManager* manager);

		virtual ~RendererComponent();

		void PreRender(Renderer* renderer) override;

		void Render(Renderer* renderer) const override;
		void BatchRender(uint32_t instance_index = 0) const override;

		Material* getMaterial() const { return material.pointer; }
		MaterialHandle getMaterialHandle() const { return material; }

		void setMaterial(MaterialHandle handle) { this->material.handle = material.handle; this->material.pointer = material.pointer; }

		static ComponentType getStaticComponentType() { return ComponentType::RendererComponent; }

	private:
		MaterialHandle material;
	};
};

#endif