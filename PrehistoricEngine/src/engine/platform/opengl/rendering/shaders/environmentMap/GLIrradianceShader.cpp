#include "Includes.hpp"
#include "GLIrradianceShader.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	GLIrradianceShader::GLIrradianceShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/irradiance_CS.glsl"), COMPUTE_SHADER);

		CompileShader();

		AddUniform("environmentMap");
		AddUniform("resolution");
		AddUniform("resolution_irradianceMap");
	}

	void GLIrradianceShader::UpdateUniforms(Texture* texture) const
	{
		texture->Bind();
		SetUniformi("environmentMap", 0);

		SetUniformf("resolution", (float)__EnvironmentMapConfig.environmentMapResolution);
		SetUniformf("resolution_irradianceMap", (float)__EnvironmentMapConfig.irradianceMapResolution);
	}
};
