#include "engine/prehistoric/core/util/Includes.hpp"
#include "Renderer.h"

#include "engine/prehistoric/core/node/component/renderer/RenderableComponent.h"

#include "engine/prehistoric/core/node/component/light/Light.h"

Renderer::Renderer(Window* window, Camera* camera)
	: window(window), camera(camera), wireframeMode(false)
{
}

void register_model(std::unordered_map<Pipeline*, std::vector<RenderableComponent*>>& map, RenderableComponent* renderable)
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
		register_model(models_2d, renderable);
		break;
	default:
		break;
	}
}

void Renderer::AddLight(Light* light)
{
	if (light->isSun())
		sun = light;

	lights.push_back(light);
}
