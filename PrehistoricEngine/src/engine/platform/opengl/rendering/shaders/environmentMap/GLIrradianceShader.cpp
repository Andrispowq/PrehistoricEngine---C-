#include "Includes.hpp"
#include "GLIrradianceShader.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"

namespace Prehistoric
{
	GLIrradianceShader::GLIrradianceShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/irradiance_CS.glsl"), COMPUTE_SHADER);

		CompileShader();

		AddUniform("resolution");
		AddUniform("environmentMap");
	}

	void GLIrradianceShader::UpdateUniforms(Texture* texture) const
	{
		texture->Bind(0);
		SetUniformi("environmentMap", 0);

		SetUniformf("resolution", (float)EnvironmentMapConfig::irradianceMapResolution);
	}
};
