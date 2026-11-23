#include "Includes.hpp"
#include "GLVolumetricPostProcessingShader.h"

#include "prehistoric/application/Application.h"
#include "prehistoric/common/buffer/UniformBufferObject.h"

#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric
{
	GLVolumetricPostProcessingShader::GLVolumetricPostProcessingShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/volumetric_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("scene");
		AddUniform("positionMetallicImage");
		AddUniform("albedoRoughnessImage");
		AddUniform("normalImage");
		AddShadowUniforms();

		AddUniform("resolution");
		AddUniform("cameraPosition");

		AddUniform("sunPosition");
		AddUniform("sunColour");


		AddUniform("m_view");

	}

	void GLVolumetricPostProcessingShader::UpdateUniforms(Texture* scene,
		Texture* positionMetallic, Texture* albedoRoughness, Texture* normal,
		Renderer* renderer) const
	{
		Camera* camera = renderer->getCamera();
		scene->Bind(0);
		SetUniformi("scene", 0);
		positionMetallic->Bind(1);
		SetUniformi("positionMetallicImage", 1);
		albedoRoughness->Bind(2);
		SetUniformi("albedoRoughnessImage", 2);
		normal->Bind(3);
		SetUniformi("normalImage", 3);
		BindShadowUniforms(renderer, 4);

		SetUniform("resolution", Vector2f((float)__FrameworkConfig.windowWidth, (float)__FrameworkConfig.windowHeight));
		SetUniform("cameraPosition", camera->getPosition());

		for (auto light : renderer->getLights())
		{
			if (light->castShadows())
			{
				SetUniform("sunPosition", light->getParent()->getWorldTransform().getPosition());
				SetUniform("sunColour", light->getColour());
			}
		}

		SetUniform("m_view", camera->getViewMatrix());
	}
}
