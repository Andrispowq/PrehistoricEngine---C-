#include "Includes.hpp"
#include "GLInversionShader.h"

namespace Prehistoric
{
	GLInversionShader::GLInversionShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/fft/inversion_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("N");
		AddUniform("pingpong");
	}

	void GLInversionShader::UpdateUniforms(int N, int pingpong) const
	{
		SetUniformi("N", N);
		SetUniformi("pingpong", pingpong);
	}
}