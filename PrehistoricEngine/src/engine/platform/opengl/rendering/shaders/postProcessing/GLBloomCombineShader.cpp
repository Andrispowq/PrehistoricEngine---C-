#include "Includes.hpp"
#include "GLBloomCombineShader.h"

namespace Prehistoric
{
	GLBloomCombineShader::GLBloomCombineShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/bloom_combine_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("scene");
		AddUniform("bloom");

		AddUniform("bloom_factor");
		AddUniform("screenSize");
	}

	void GLBloomCombineShader::UpdateUniforms(Texture* scene, Texture* bloom, float bloom_factor) const
	{
		scene->Bind(0);
		SetUniformi("scene", 0);

		bloom->Bind(1);
		SetUniformi("bloom", 1);

		SetUniformf("bloom_factor", bloom_factor);
		SetUniform("screenSize", Vector2f((float)FrameworkConfig::windowWidth, (float)FrameworkConfig::windowHeight));
	}
};