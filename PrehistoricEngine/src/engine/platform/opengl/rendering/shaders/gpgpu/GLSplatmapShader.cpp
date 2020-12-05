#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLSplatMapShader.h"

namespace Prehistoric
{
	GLSplatMapShader::GLSplatMapShader()
		: GLShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/gpgpu/splatMap_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("normalmap");
		AddUniform("N");
	}

	void GLSplatMapShader::UpdateUniforms(Texture* texture, uint32_t N) const
	{
		texture->Bind();
		SetUniformi("normalmap", 0);

		SetUniformi("N", N);
	}
};
