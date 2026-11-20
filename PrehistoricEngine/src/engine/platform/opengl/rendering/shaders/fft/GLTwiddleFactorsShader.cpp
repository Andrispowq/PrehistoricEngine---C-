#include "Includes.hpp"
#include "GLTwiddleFactorsShader.h"

namespace Prehistoric
{
	GLTwiddleFactorsShader::GLTwiddleFactorsShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/fft/twiddleFactors_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("N");
	}

	void GLTwiddleFactorsShader::UpdateUniforms(int N) const
	{
		SetUniformi("N", N);
	}
}