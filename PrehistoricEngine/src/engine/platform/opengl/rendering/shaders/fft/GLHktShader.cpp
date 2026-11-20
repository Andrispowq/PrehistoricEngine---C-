#include "Includes.hpp"
#include "GLHktShader.h"

namespace Prehistoric
{
	GLHktShader::GLHktShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/fft/hkt_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("L");
		AddUniform("N");
		AddUniform("t");
	}

	void GLHktShader::UpdateUniforms(int N, int L, float t) const
	{
		SetUniformi("N", N);
		SetUniformi("L", L);
		SetUniformf("t", t);
	}
}