#include "Includes.hpp"
#include "GLPrefilterShader.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	GLPrefilterShader::GLPrefilterShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/prefilter_CS.glsl"), COMPUTE_SHADER);

		CompileShader();

		AddUniform("environmentMap");
		AddUniform("roughness");
		AddUniform("resolution");
		AddUniform("resolution_environmentMap");
	}

	void GLPrefilterShader::UpdateUniforms(Texture* texture, float roughness, float resolution) const
	{
		texture->Bind(0);
		SetUniformi("environmentMap", 0);

		SetUniformf("roughness", roughness);
		SetUniformf("resolution", resolution);
		SetUniformf("resolution_environmentMap", (float)__EnvironmentMapConfig.environmentMapResolution);
	}
};
