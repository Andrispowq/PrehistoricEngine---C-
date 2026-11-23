#include "Includes.hpp"
#include "GLAtmosphereScatteringShader.h"

#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/core/modules/atmosphere/AtmosphereCapture.h"
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

	void GLAtmosphereScatteringShader::UpdateUniforms(const EnvironmentCaptureSource& source, const Matrix4f& transform, const Matrix4f& view,
		const Matrix4f& projection, bool capture) const
	{
		SetUniform("m_transform", transform);
		const auto& sky = source.sky;
		auto xyz = [](const std::array<float, 4>& v) { return Vector3f(v[0], v[1], v[2]); };
		SetUniform("sunPosition", xyz(sky.sunPositionRadius));

		SetUniform("m_view", view);
		SetUniform("m_projection", projection);

		SetUniformf("sunRadius", sky.sunPositionRadius[3]);
		SetUniform("sunColour", xyz(sky.sunColourIntensity));
		SetUniformf("sunIntensity", sky.sunColourIntensity[3]);

		SetUniformf("planetRadius", sky.radiiAndScaleHeights[0]);
		SetUniformf("atmosphereRadius", sky.radiiAndScaleHeights[1]);

		SetUniform("rayleigh", xyz(sky.rayleighMie));
		SetUniformf("rayleighHeightScale", sky.radiiAndScaleHeights[2]);
		SetUniformf("mie", sky.rayleighMie[3]);
		SetUniformf("mieHeightScale", sky.radiiAndScaleHeights[3]);
		SetUniformf("mieDirection", sky.anisotropyAndShift[0]);

		SetUniformf("horizontalVerticalShift", sky.anisotropyAndShift[1]);
		SetUniformi("width", __FrameworkConfig.windowWidth);
		SetUniformi("height", __FrameworkConfig.windowHeight);
		SetUniformi("isReflection", !capture && __WaterConfig.stage == WaterRenderStage::Reflection ? 1 : 0);
		SetUniformi("isCubemap", capture);

		SetUniformf("exposure", source.exposure);
	}

}
