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

    void register_model(ModelBatches& batches, RenderableComponent* renderable)
    {
        auto* component = dynamic_cast<RendererComponent*>(renderable);
        if (!component) return;
        batches[renderable->getPipeline()][component->getMaterial()].push_back(renderable);
    }

    void unregister_model(ModelBatches& batches, RenderableComponent* renderable)
    {
        auto group = batches.find(renderable->getPipeline());
        if (group == batches.end()) return;
        auto* component = dynamic_cast<RendererComponent*>(renderable);
        if (!component) return;
        auto batch = group->second.find(component->getMaterial());
        if (batch == group->second.end()) return;
        auto& objects = batch->second;
        auto object = std::find(objects.begin(), objects.end(), renderable);
        if (object != objects.end()) objects.erase(object);
        if (objects.empty()) group->second.erase(batch);
        if (group->second.empty()) batches.erase(group);
    }

	void register_2d_model(std::unordered_map<Pipeline*, std::vector<RenderableComponent*>>& map, RenderableComponent* renderable)
	{
		map[renderable->getPipeline()].push_back(renderable);
	}

	void unregister_2d_model(std::unordered_map<Pipeline*, std::vector<RenderableComponent*>>& map, RenderableComponent* renderable)
	{
		Pipeline* pipeline = renderable->getPipeline();
		auto pipeIdx = map.find(pipeline);
	
		if (pipeIdx != map.end())
		{
			auto& renderables = pipeIdx->second;
			auto rendIdx = std::find(renderables.begin(), renderables.end(), renderable);
	
			if (rendIdx != renderables.end()) renderables.erase(rendIdx);
		}
	}

	void Renderer::Render()
	{
		if (!CanRenderFrame()) return;
		UpdateEnvironmentLighting();
		RenderScene();
		RenderHDR();
		FinishFrame();
	}

    void Renderer::ClearFrameSubmissions()
    {
        models_3d.clear();
        models_transparency.clear();
        models_water.clear();
        models_2d.clear();
        lights.clear();
        atmosphere = nullptr;
    }

    void Renderer::ResizePipelines(uint32_t width, uint32_t height)
    {
        for (Pipeline* pipeline : manager->get<Pipeline>())
        {
            pipeline->setViewportSize({float(width), float(height)});
            pipeline->setScissorSize({width, height});
            pipeline->RecreatePipeline();
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
		if (__FrameworkConfig.api == Vulkan && !light->illuminates()) return;
		lights.push_back(light);
	}

	void Renderer::UnregisterModel(RenderableComponent* renderable)
	{
		switch (renderable->getPriority())
		{
		case RenderPriority::_3D:
			unregister_model(models_3d, renderable);
			unregister_model(models_water, renderable);
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
