#include "Includes.hpp"
#include "Renderer.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "prehistoric/core/node/component/light/Light.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	Renderer::Renderer(Window* window, Camera* camera, AssembledAssetManager* manager)
		: window(window), camera(camera), manager(manager), wireframeMode(false)
	{
	}

	void register_model(std::unordered_map<Pipeline*, std::unordered_map<Material*, std::vector<RenderableComponent*>>>& map, RenderableComponent* renderable)
	{
		RendererComponent* comp = dynamic_cast<RendererComponent*>(renderable);
		if (!comp)
			return;

		Pipeline* pipeline = renderable->getPipeline();
		Material* material = comp->getMaterial();
		
		uint64_t h0 = 0;
		if (material != nullptr)
			h0 = material->GetTextureHash();

		auto index = map.find(pipeline);
		if (index == map.end())
		{
			std::vector<RenderableComponent*> renderers = { renderable };
			std::unordered_map<Material*, std::vector<RenderableComponent*>> matRend;
			matRend.emplace(material, renderers);
			map.emplace(pipeline, matRend);
		}
		else
		{
			auto& list = index->second;
			for (auto& mat : list)
			{
				Material* curr_mat = mat.first;
				uint64_t h1 = 0;
				if (curr_mat != nullptr)
					h1 = curr_mat->GetTextureHash();

				if (h0 == h1)
				{
					mat.second.push_back(renderable);
					break;
				}
			}

			std::vector<RenderableComponent*> renderers = { renderable };
			auto& matRend = index->second;
			matRend.emplace(material, renderers);
		}
	}

	void register_2d_model(std::unordered_map<Pipeline*, std::vector<RenderableComponent*>>& map, RenderableComponent* renderable)
	{
		Pipeline* pipeline = renderable->getPipeline();

		if (map.find(pipeline) != map.end())
		{
			auto& renderables = map[pipeline];
			renderables.push_back(renderable);
		}
		else
		{
			std::vector<RenderableComponent*> renderers = { renderable };
			map.insert(std::make_pair(pipeline, renderers));
		}
	}

	void Renderer::AddModel(RenderableComponent* renderable)
	{
		switch (renderable->getPriority())
		{
		case RenderPriority::_3D:
			register_model(models_3d, renderable);
			break;
		case RenderPriority::_TRANSPARENCY:
			register_model(models_transparency, renderable);
			break;
		case RenderPriority::_2D:
			register_2d_model(models_2d, renderable);
			break;
		default:
			break;
		}
	}

	void Renderer::AddLight(Light* light)
	{
		lights.push_back(light);
	}
};
