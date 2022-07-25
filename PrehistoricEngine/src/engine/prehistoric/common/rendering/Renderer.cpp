#include "Includes.hpp"
#include "Renderer.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "prehistoric/core/node/component/light/Light.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/core/config/EngineSettings.h"

#include "prehistoric/application/Application.h"

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
			h0 = material->GetHash();

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
					h1 = curr_mat->GetHash();

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

	void unregister_model(std::unordered_map<Pipeline*, std::unordered_map<Material*, std::vector<RenderableComponent*>>>& map, RenderableComponent* renderable)
	{
		RendererComponent* comp = dynamic_cast<RendererComponent*>(renderable);
		if (!comp)
			return;

		Pipeline* pipeline = renderable->getPipeline();
		Material* material = comp->getMaterial();

		auto index = map.find(pipeline);
		if (index != map.end())
		{
			auto& list = index->second;
			auto matIdx = list.find(material);

			if (matIdx != list.end())
			{
				auto& rend = matIdx->second;
				auto rendIdx = std::find(rend.begin(), rend.end(), renderable);

				if (rendIdx != rend.end())
				{
					rend.erase(rendIdx);
				}
			}
		}
	}

	void register_2d_model(std::unordered_map<Pipeline*, std::vector<RenderableComponent*>>& map, RenderableComponent* renderable)
	{
		Pipeline* pipeline = renderable->getPipeline();
		auto pipeIdx = map.find(pipeline);

		if (pipeIdx != map.end())
		{
			auto& renderables = pipeIdx->second;
			renderables.push_back(renderable);
		}
		else
		{
			std::vector<RenderableComponent*> renderers = { renderable };
			map.insert(std::make_pair(pipeline, renderers));
		}
	}

	void unregister_2d_model(std::unordered_map<Pipeline*, std::vector<RenderableComponent*>>& map, RenderableComponent* renderable)
	{
		Pipeline* pipeline = renderable->getPipeline();
		auto pipeIdx = map.find(pipeline);
	
		if (pipeIdx != map.end())
		{
			auto& renderables = pipeIdx->second;
			auto rendIdx = std::find(renderables.begin(), renderables.end(), renderable);
	
			renderables.erase(rendIdx);
		}
	}

	void Renderer::RegisterModel(RenderableComponent* renderable)
	{
		switch (renderable->getPriority())
		{
		case RenderPriority::_3D:
			register_model(models_3d, renderable);
			if (renderable->hasReflection())
			{
				register_model(models_water, renderable);
			}
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

	void Renderer::RegisterLight(Light* light)
	{
		lights.push_back(light);
	}

	void Renderer::UnregisterModel(RenderableComponent* renderable)
	{
		switch (renderable->getPriority())
		{
		case RenderPriority::_3D:
			unregister_model(models_3d, renderable);
			break;
		case RenderPriority::_TRANSPARENCY:
			unregister_model(models_transparency, renderable);
			break;
		case RenderPriority::_2D:
			unregister_2d_model(models_2d, renderable);
			break;
		default:
			break;
		}
	}

	void Renderer::UnregisterLight(Light* light)
	{
		auto idx = std::find(lights.begin(), lights.end(), light);
		if (idx != lights.end())
		{
			lights.erase(idx);
		}
	}
};
