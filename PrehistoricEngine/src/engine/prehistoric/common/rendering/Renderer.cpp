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

		auto index = map.find(pipeline);
		if (index != map.end())
		{
			auto& list = index->second;
			for (auto& elem : list)
			{
				Material* curr_mat = elem.first;

				if (curr_mat != nullptr && material != nullptr &&
					curr_mat != material)
				{
					for (auto& ptr : material->getTextures())
					{
						if (curr_mat->getTexture(ptr.first) != ptr.second.pointer)
						{
							break;
						}
					}

					if (curr_mat->getTextures().size() != material->getTextures().size())
					{
						break;
					}
				}
				else if(curr_mat == material)
				{
					auto& renderables = elem.second;
					renderables.push_back(renderable);
				}
				else
				{
					break;
				}

				auto& renderables = elem.second;
				renderables.push_back(renderable);
				return;
			}

			std::vector<RenderableComponent*> renderers = { renderable };
			index->second.emplace(material, renderers);
		}
		else
		{
			std::vector<RenderableComponent*> renderers = { renderable };
			std::unordered_map<Material*, std::vector<RenderableComponent*>> matRend;
			matRend.emplace(material, renderers);
			map.emplace(pipeline, matRend);
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
