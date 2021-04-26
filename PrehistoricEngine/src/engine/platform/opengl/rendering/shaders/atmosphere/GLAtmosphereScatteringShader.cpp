#include "Includes.hpp"
#include "GLAtmosphereScatteringShader.h"

#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/core/modules/atmosphere/Atmosphere.h"

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

		SetUniformf("sunRadius", AtmosphereConfig::sunRadius);
		SetUniform("sunColour", AtmosphereConfig::sunColour);
		SetUniformf("sunIntensity", AtmosphereConfig::sunIntensity);

		SetUniformf("planetRadius", AtmosphereConfig::planetRadius);
		SetUniformf("atmosphereRadius", AtmosphereConfig::atmosphereRadius);

		SetUniform("rayleigh", AtmosphereConfig::rayleigh);
		SetUniformf("rayleighHeightScale", AtmosphereConfig::rayleighHeightScale);
		SetUniformf("mie", AtmosphereConfig::mie);
		SetUniformf("mieHeightScale", AtmosphereConfig::mieHeightScale);
		SetUniformf("mieDirection", AtmosphereConfig::mieDirection);

		SetUniformf("horizontalVerticalShift", AtmosphereConfig::horizontalVerticalShift);
		SetUniformi("width", FrameworkConfig::windowWidth);
		SetUniformi("height", FrameworkConfig::windowHeight);
		SetUniformi("isReflection", 0); //TODO: getting if it's a reflection or not from the RenderingEngine
		SetUniformi("isCubemap", 1);

		SetUniformf("exposure", EngineConfig::rendererExposure);
	}

	void GLAtmosphereScatteringShader::UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const
	{
		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());

		SetUniformf("sunRadius", AtmosphereConfig::sunRadius);
		SetUniform("sunColour", AtmosphereConfig::sunColour);
		SetUniformf("sunIntensity", AtmosphereConfig::sunIntensity);

		SetUniformf("planetRadius", AtmosphereConfig::planetRadius);
		SetUniformf("atmosphereRadius", AtmosphereConfig::atmosphereRadius);

		SetUniform("rayleigh", AtmosphereConfig::rayleigh);
		SetUniformf("rayleighHeightScale", AtmosphereConfig::rayleighHeightScale);
		SetUniformf("mie", AtmosphereConfig::mie);
		SetUniformf("mieHeightScale", AtmosphereConfig::mieHeightScale);
		SetUniformf("mieDirection", AtmosphereConfig::mieDirection);

		SetUniformf("horizontalVerticalShift", AtmosphereConfig::horizontalVerticalShift);
		SetUniformi("width", FrameworkConfig::windowWidth);
		SetUniformi("height", FrameworkConfig::windowHeight);
		SetUniformi("isReflection", 0); //TODO: getting if it's a reflection or not from the RenderingEngine
		SetUniformi("isCubemap", 0);

		SetUniformf("exposure", EngineConfig::rendererExposure);
	}

	void GLAtmosphereScatteringShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix());
		SetUniform("sunPosition", ((Atmosphere*)object)->getSun()->getParent()->getWorldTransform().getPosition());
	}
};
