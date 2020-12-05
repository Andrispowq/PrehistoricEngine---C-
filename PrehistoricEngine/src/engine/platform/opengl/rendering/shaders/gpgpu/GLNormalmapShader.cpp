#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLNormalMapShader.h"

namespace Prehistoric
{
	GLNormalMapShader::GLNormalMapShader()
		: GLShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/gpgpu/normalMap_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("heightmap");
		AddUniform("N");
		AddUniform("strength");
	}

	void GLNormalMapShader::UpdateUniforms(Texture* texture, uint32_t N, float strength) const
	{
		texture->Bind();
		SetUniformi("heightmap", 0);

		SetUniformi("N", N);
		SetUniformf("strength", strength);
	}
};
