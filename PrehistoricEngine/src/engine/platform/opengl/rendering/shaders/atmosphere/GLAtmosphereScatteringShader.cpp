#include "Includes.hpp"
#include "GLAtmosphereScatteringShader.h"

#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/core/modules/atmosphere/Atmosphere.h"
#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	GLAtmosphereScatteringShader::GLAtmosphereScatteringShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/atmosphere/scattering_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/atmosphere/scattering_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		AddUniform("m_transform");
		AddUniform("m_view");
		AddUniform("m_projection");

		AddUniform("sunPosition");
		AddUniform("sunRadius");
		AddUniform("sunColour");
		AddUniform("sunIntensity");

		AddUniform("planetRadius");
		AddUniform("atmosphereRadius");

		AddUniform("rayleigh");
		AddUniform("rayleighHeightScale");
		AddUniform("mie");
		AddUniform("mieHeightScale");
		AddUniform("mieDirection");

		AddUniform("horizontalVerticalShift");
		AddUniform("width");
		AddUniform("height");
		AddUniform("isReflection");
		AddUniform("isCubemap");

		AddUniform("exposure");
	}

	void GLAtmosphereScatteringShader::UpdateUniforms(GameObject* atmosphere, Matrix4f view, Matrix4f projection) const
	{
		SetUniform("m_transform", Matrix4f::Identity());
		SetUniform("sunPosition", ((Atmosphere*)atmosphere)->getSun()->getParent()->getWorldTransform().getPosition());

		SetUniform("m_view", view);
		SetUniform("m_projection", projection);

		SetUniformf("sunRadius", __AtmosphereConfig.sunRadius);
		SetUniform("sunColour", __AtmosphereConfig.sunColour);
		SetUniformf("sunIntensity", __AtmosphereConfig.sunIntensity);

		SetUniformf("planetRadius", __AtmosphereConfig.planetRadius);
		SetUniformf("atmosphereRadius", __AtmosphereConfig.atmosphereRadius);

		SetUniform("rayleigh", __AtmosphereConfig.rayleigh);
		SetUniformf("rayleighHeightScale", __AtmosphereConfig.rayleighHeightScale);
		SetUniformf("mie", __AtmosphereConfig.mie);
		SetUniformf("mieHeightScale", __AtmosphereConfig.mieHeightScale);
		SetUniformf("mieDirection", __AtmosphereConfig.mieDirection);

		SetUniformf("horizontalVerticalShift", __AtmosphereConfig.horizontalVerticalShift);
		SetUniformi("width", __FrameworkConfig.windowWidth);
		SetUniformi("height", __FrameworkConfig.windowHeight);
		SetUniformi("isReflection", 0); //TODO: getting if it's a reflection or not from the RenderingEngine
		SetUniformi("isCubemap", 1);

		SetUniformf("exposure", __EngineConfig.rendererExposure);
	}

	void GLAtmosphereScatteringShader::UpdateGlobalUniforms(Renderer* renderer) const
	{
		Camera* camera = renderer->getCamera();

		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());

		SetUniformf("sunRadius", __AtmosphereConfig.sunRadius);
		SetUniform("sunColour", __AtmosphereConfig.sunColour);
		SetUniformf("sunIntensity", __AtmosphereConfig.sunIntensity);

		SetUniformf("planetRadius", __AtmosphereConfig.planetRadius);
		SetUniformf("atmosphereRadius", __AtmosphereConfig.atmosphereRadius);

		SetUniform("rayleigh", __AtmosphereConfig.rayleigh);
		SetUniformf("rayleighHeightScale", __AtmosphereConfig.rayleighHeightScale);
		SetUniformf("mie", __AtmosphereConfig.mie);
		SetUniformf("mieHeightScale", __AtmosphereConfig.mieHeightScale);
		SetUniformf("mieDirection", __AtmosphereConfig.mieDirection);

		SetUniformf("horizontalVerticalShift", __AtmosphereConfig.horizontalVerticalShift);
		SetUniformi("width", __FrameworkConfig.windowWidth);
		SetUniformi("height", __FrameworkConfig.windowHeight);
		SetUniformi("isReflection", 0); //TODO: getting if it's a reflection or not from the RenderingEngine
		SetUniformi("isCubemap", 0);

		SetUniformf("exposure", __EngineConfig.rendererExposure);
	}

	void GLAtmosphereScatteringShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix());
		SetUniform("sunPosition", ((Atmosphere*)object)->getSun()->getParent()->getWorldTransform().getPosition());
	}
};
