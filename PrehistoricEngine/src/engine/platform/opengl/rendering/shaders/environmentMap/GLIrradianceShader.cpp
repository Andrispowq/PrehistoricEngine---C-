#include "Includes.hpp"
#include "GLIrradianceShader.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"

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

		SetUniformf("resolution", (float)EnvironmentMapConfig::environmentMapResolution);
		SetUniformf("resolution_irradianceMap", (float)EnvironmentMapConfig::irradianceMapResolution);
	}
};
