#include "Includes.hpp"
#include "GLEnvironmentMapShader.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"

namespace Prehistoric
{
	GLEnvironmentMapShader::GLEnvironmentMapShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/environmentMap_CS.glsl"), COMPUTE_SHADER);

		CompileShader();

		AddUniform("equirectangularMapResolution");
		AddUniform("resolution");
	}

	void GLEnvironmentMapShader::UpdateUniforms(Vector2f equirectMapRes) const
	{
		SetUniform("equirectangularMapResolution", equirectMapRes);
		SetUniformf("resolution", (float)EnvironmentMapConfig::environmentMapResolution);
	}
};
