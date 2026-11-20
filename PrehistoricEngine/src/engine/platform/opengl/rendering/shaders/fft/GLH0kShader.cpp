#include "Includes.hpp"
#include "GLH0kShader.h"

namespace Prehistoric
{
	GLH0kShader::GLH0kShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/fft/h0k_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("N");
		AddUniform("L");
		AddUniform("amplitude");
		AddUniform("direction");
		AddUniform("intensity");
		AddUniform("l");
		AddUniform("alignment");

		AddUniform("noise_r0");
		AddUniform("noise_i0");
		AddUniform("noise_r1");
		AddUniform("noise_i1");
	}

	void GLH0kShader::UpdateUniforms(int N, int L, float amplitude, Vector2f direction, float alignment, float intensity, float capillarSupressFactor) const
	{
		SetUniformi("N", N);
		SetUniformi("L", L);
		SetUniformf("amplitude", amplitude);
		SetUniformf("intensity", intensity);
		SetUniform("direction", direction);
		SetUniformf("l", capillarSupressFactor);
		SetUniformf("alignment", alignment);
	}

	void GLH0kShader::UpdateUniformsTextures(int t0, int t1, int t2, int t3) const
	{
		SetUniformi("noise_r0", t0);
		SetUniformi("noise_i0", t1);
		SetUniformi("noise_r1", t2);
		SetUniformi("noise_i1", t3);
	}
}