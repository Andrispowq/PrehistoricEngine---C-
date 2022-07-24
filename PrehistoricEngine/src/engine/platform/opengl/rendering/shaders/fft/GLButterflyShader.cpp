#include "Includes.hpp"
#include "GLButterflyShader.h"

namespace Prehistoric
{
	GLButterflyShader::GLButterflyShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/fft/butterfly_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("stage");
		AddUniform("pingpong");
		AddUniform("direction");
	}

	void GLButterflyShader::UpdateUniforms(int pingpong, int direciton, int stage) const
	{
		SetUniformi("stage", stage);
		SetUniformi("pingpong", pingpong);
		SetUniformi("direction", direciton);
	}
}