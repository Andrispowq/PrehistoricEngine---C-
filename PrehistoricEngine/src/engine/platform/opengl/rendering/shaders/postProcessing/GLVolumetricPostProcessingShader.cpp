#include "Includes.hpp"
#include "GLVolumetricPostProcessingShader.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"
#include "prehistoric/common/buffer/UniformBufferObject.h"

#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric
{
	extern UniformBufferObject* _matrices;
	extern Texture* _shadowMap;
	extern std::vector<float> cascadeDistances;

	GLVolumetricPostProcessingShader::GLVolumetricPostProcessingShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/volumetric_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("scene");
		AddUniform("positionMetallicImage");
		AddUniform("albedoRoughnessImage");
		AddUniform("normalImage");
		AddUniform("shadowMap");

		AddUniform("resolution");
		AddUniform("cameraPosition");

		AddUniform("sunPosition");
		AddUniform("sunColour");

		AddUniform("cascadeCount");
		AddUniform("farPlane");

		AddUniform("m_view");
		AddUniform("LightSpaceMatrices");

		for (uint32_t i = 0; i < cascadeDistances.size(); i++)
			AddUniform("cascadePlaneDistances[" + std::to_string(i) + "]");
	}

	void GLVolumetricPostProcessingShader::UpdateUniforms(Texture* scene, Texture* positionMetallic, Texture* albedoRoughness, Texture* normal, Camera* camera, std::vector<Light*>& lights) const
	{
		scene->Bind(0);
		SetUniformi("scene", 0);
		positionMetallic->Bind(1);
		SetUniformi("positionMetallicImage", 1);
		albedoRoughness->Bind(2);
		SetUniformi("albedoRoughnessImage", 2);
		normal->Bind(3);
		SetUniformi("normalImage", 3);
		if(_shadowMap) _shadowMap->Bind(4);
		SetUniformi("shadowMap", 4);

		SetUniform("resolution", Vector2f((float)FrameworkConfig::windowWidth, (float)FrameworkConfig::windowHeight));
		SetUniform("cameraPosition", camera->getPosition());

		for (auto light : lights)
		{
			if (light->castShadows())
			{
				SetUniform("sunPosition", light->getParent()->getWorldTransform().getPosition());
				SetUniform("sunColour", light->getColour());
			}
		}

		SetUniformi("cascadeCount", (int)cascadeDistances.size());
		SetUniformf("farPlane", EngineConfig::rendererFarPlane);

		_matrices->BindBase(nullptr, 0);
		SetUniform("m_view", camera->getViewMatrix());
		SetUniformi("LightSpaceMatrices", 0);

		for (uint32_t i = 0; i < cascadeDistances.size(); i++)
			SetUniformf("cascadePlaneDistances[" + std::to_string(i) + "]", cascadeDistances[i]);
 	}
};
