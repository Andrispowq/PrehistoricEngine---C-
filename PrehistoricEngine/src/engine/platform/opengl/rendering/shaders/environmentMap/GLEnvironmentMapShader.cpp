#include "Includes.hpp"
#include "GLEnvironmentMapShader.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"

namespace Prehistoric
{
	GLEnvironmentMapShader::GLEnvironmentMapShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/environmentMap_CS.glsl"), COMPUTE_SHADER);

		CompileShader();

		AddUniform("equirectangularMap");
		AddUniform("resolution");
	}

	void GLEnvironmentMapShader::UpdateUniforms(Texture* texture) const
	{
		texture->Bind();
		SetUniformi("equirectangularMap", 0);

		SetUniformf("resolution", (float)EnvironmentMapConfig::environmentMapResolution);
	}
};
