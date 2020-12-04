#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLRenderer.h"

#include "engine/prehistoric/core/node/component/renderer/RenderableComponent.h"

void GLRenderer::PrepareRendering()
{
}

void GLRenderer::EndRendering()
{
	window->Render(nullptr);

	//Clear the lists for the next iteration
	models_3d.clear();
	models_transparency.clear();
	models_2d.clear();
	lights.clear();
}

void GLRenderer::Render()
{
	for (auto pipeline : models_3d)
	{
		Pipeline* pl = pipeline.first;

		pl->BindPipeline(nullptr);
		pl->getShader()->UpdateShaderUniforms(camera, lights);
		pl->getShader()->UpdateSharedUniforms(pipeline.second[0]->getParent()); //Safe -> there is at least 1 element in the array

		for (auto renderer : pipeline.second)
		{
			renderer->BatchRender();
		}

		pl->UnbindPipeline();
	}

	//TODO: enable alpha blending
	for (auto pipeline : models_transparency)
	{
		Pipeline* pl = pipeline.first;

		pl->BindPipeline(nullptr);
		pl->getShader()->UpdateShaderUniforms(camera, lights);
		pl->getShader()->UpdateSharedUniforms(pipeline.second[0]->getParent()); //Safe -> there is at least 1 element in the array

		for (auto renderer : pipeline.second)
		{
			renderer->BatchRender();
		}

		pl->UnbindPipeline();
	}

	//TODO: disable alpha blending and depth testing
	for (auto pipeline : models_2d)
	{
		Pipeline* pl = pipeline.first;

		pl->BindPipeline(nullptr);

		for (auto renderer : pipeline.second)
		{
			renderer->BatchRender();
		}

		pl->UnbindPipeline();
	}
}
