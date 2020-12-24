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
		RendererComponent(size_t pipelineID, size_t materialID, Window* window, AssembledAssetManager* manager);
		RendererComponent(Window* window, AssembledAssetManager* manager);

		virtual ~RendererComponent();

		void PreRender(Renderer* renderer) override;

		void Render(Renderer* renderer) const override;
		void BatchRender(uint32_t instance_index = 0) const override;

		inline size_t getMaterialIndex() const { return materialIndex; }
		Material* getMaterial() const { return material; }

		static ComponentType getStaticComponentType() { return ComponentType::RendererComponent; }

	private:
		size_t materialIndex;
		Material* material;
	};
};

#endif