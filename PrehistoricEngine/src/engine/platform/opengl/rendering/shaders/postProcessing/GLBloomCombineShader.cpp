#include "Includes.hpp"
#include "GLBloomCombineShader.h"

namespace Prehistoric
{
	GLBloomCombineShader::GLBloomCombineShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/bloom_combine_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("scene");

		AddUniform("bloom0");
		AddUniform("bloom1");
		AddUniform("bloom2");
		AddUniform("bloom3");
		AddUniform("bloom4");

		AddUniform("bloom_factor");
		AddUniform("screenSize");
	}

	void GLBloomCombineShader::UpdateUniforms(Texture* scene, Texture* bloom[5], float bloom_factor) const
	{
		scene->Bind(0);
		SetUniformi("scene", 0);

		bloom[0]->Bind(1);
		SetUniformi("bloom0", 1);
		bloom[1]->Bind(2);
		SetUniformi("bloom1", 2);
		bloom[2]->Bind(3);
		SetUniformi("bloom2", 3);
		bloom[3]->Bind(4);
		SetUniformi("bloom3", 4);
		bloom[4]->Bind(5);
		SetUniformi("bloom4", 5);

		SetUniformf("bloom_factor", bloom_factor);
		SetUniform("screenSize", Vector2f((float)FrameworkConfig::windowWidth, (float)FrameworkConfig::windowHeight));
	}
};