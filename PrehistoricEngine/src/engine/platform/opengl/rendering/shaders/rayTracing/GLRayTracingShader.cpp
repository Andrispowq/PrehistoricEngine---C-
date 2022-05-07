#include "Includes.hpp"
#include "GLRayTracingShader.h"

#include "prehistoric/application/Application.h"

#include "platform/opengl/rendering/GLRenderer.h"

namespace Prehistoric
{
	GLRayTracingShader::GLRayTracingShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/rayTracing/pathTracing_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("environment");

		AddUniform("Projection");
		AddUniform("InvProjection");
		AddUniform("NearFar");
		AddUniform("View");
		AddUniform("InvView");
		AddUniform("ProjectionView");
		AddUniform("ViewPos");
		AddUniform("ViewDir");

		AddUniform("current_spp");
		AddUniform("blendFactor");
	}

	void GLRayTracingShader::UpdateUniforms(Renderer* renderer, uint32_t current_spp) const
	{
		Camera* camera = renderer->getCamera();

		__EnvironmentMapConfig.environmentMap->Bind(0);
		SetUniformi("environment", 0);

		SetUniform("Projection", camera->getProjectionMatrix());
		SetUniform("InvProjection", camera->getProjectionMatrix().Invert());
		SetUniform("NearFar", Vector2f(__EngineConfig.rendererNearPlane, __EngineConfig.rendererFarPlane));
		SetUniform("View", camera->getViewMatrix());
		SetUniform("InvView", camera->getViewMatrix().Invert());
		SetUniform("ProjectionView", camera->getViewProjectionMatrix());
		SetUniform("ViewPos", camera->getPosition());
		SetUniform("ViewDir", camera->getForward());

		float blend = float(current_spp) / float(current_spp + 1);
		SetUniformi("current_spp", current_spp);
		SetUniformf("blendFactor", blend);
	}
};
