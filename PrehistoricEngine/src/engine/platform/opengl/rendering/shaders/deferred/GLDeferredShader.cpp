#include "Includes.hpp"
#include "GLDeferredShader.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"
#include "platform/opengl/rendering/GLRenderer.h"

#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric
{
	GLDeferredShader::GLDeferredShader() : GLShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/deferred/deferred_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("positionMetallic");
		AddUniform("albedoRoughness");
		AddUniform("normalLit");
		AddUniform("emissionExtra");

		AddUniform("alphaCoverage");

		AddUniform("irradianceMap");
		AddUniform("prefilterMap");
		AddUniform("brdfLUT");

		AddUniform("cameraPosition");
		AddUniform("dimension");
		AddUniform("samples");
		AddUniform("max_reflection_lod");

		AddUniform("gamma");
		AddUniform("exposure");
		AddUniform("numberOfLights");

		for (uint32_t i = 0; i < EngineConfig::lightsMaxNumber; i++)
		{
			std::string name = "lights[" + std::to_string(i);

			AddUniform(name + "].position");
			AddUniform(name + "].colour");
			AddUniform(name + "].intensity");
		}
	}

	void GLDeferredShader::UpdateUniforms(Renderer* renderer, Camera* camera, const std::vector<Light*>& lights) const
	{
		GLRenderer* rend = (GLRenderer*)renderer;
		rend->getPositionMetallic()->Bind(0);
		rend->getAlbedoRoughness()->Bind(1);
		rend->getNormalLit()->Bind(2);
		rend->getEmissionExtra()->Bind(3);

		rend->getAlphaCoverage()->Bind(4);

		SetUniformi("positionMetallic", 0);
		SetUniformi("albedoRoughness", 1);
		SetUniformi("normalLit", 2);
		SetUniformi("emissionExtra", 3);

		SetUniformi("alphaCoverage", 4);

		EnvironmentMapConfig::irradianceMap->Bind(5);
		EnvironmentMapConfig::prefilterMap->Bind(6);
		EnvironmentMapConfig::brdfLUT->Bind(7);

		SetUniformi("irradianceMap", 5);
		SetUniformi("prefilterMap", 6);
		SetUniformi("brdfLUT", 7);

		SetUniform("cameraPosition", camera->getPosition());
		Window* window = renderer->getWindow();
		SetUniform("dimension", Vector2f((float)window->getWidth(), (float)window->getHeight()));
		SetUniformi("samples", FrameworkConfig::windowNumSamples);
		SetUniformf("max_reflection_lod", EnvironmentMapConfig::prefilterLevels - 1.0f);

		SetUniformf("gamma", EngineConfig::rendererGamma);
		SetUniformf("exposure", EngineConfig::rendererExposure);

		uint32_t nOfLights = EngineConfig::lightsMaxNumber > (uint32_t)lights.size() ? (uint32_t)lights.size() : EngineConfig::lightsMaxNumber;
		SetUniformi("numberOfLights", nOfLights);
#if defined(PR_DEBUG)
		for (unsigned int i = 0; i < EngineConfig::lightsMaxNumber; i++)
		{
			std::string uniformName = "lights[" + std::to_string(i) + "].";

			if (i < lights.size())
			{
				Light* light = lights[i];

				SetUniform(uniformName + "position", light->getParent()->getWorldTransform().getPosition());
				SetUniform(uniformName + "colour", light->getColour());
				SetUniform(uniformName + "intensity", light->getIntensity());
			}
			else
			{
				//Load some dummy values for debug mode so we don't access undefined memory while debugging, but we won't in release mode
				SetUniform(uniformName + "position", Vector3f());
				SetUniform(uniformName + "colour", Vector3f());
				SetUniform(uniformName + "intensity", Vector3f());
			}
		}
#else
		for (unsigned int i = 0; i < nOfLights; i++)
		{
			std::string uniformName = "lights[" + std::to_string(i) + "].";

			Light* light = lights[i];

			SetUniform(uniformName + "position", light->getParent()->getWorldTransform().getPosition());
			SetUniform(uniformName + "colour", light->getColour());
			SetUniform(uniformName + "intensity", light->getIntensity());
		}
#endif
	}
};