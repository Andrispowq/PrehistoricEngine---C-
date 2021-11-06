#include "Includes.hpp"
#include "GLBloomCombineShader.h"

namespace Prehistoric
{
	GLBloomCombineShader::GLBloomCombineShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/bloom_combine_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("image0");
		AddUniform("image1");
		AddUniform("targetSize");
	}

	void GLBloomCombineShader::UpdateUniforms(Texture* tex0, Texture* tex1, Vector2f targetSize) const
	{
		tex0->Bind(0);
		SetUniformi("image0", 0);
		tex1->Bind(1);
		SetUniformi("image1", 1);

		SetUniform("targetSize", targetSize);
	}
};