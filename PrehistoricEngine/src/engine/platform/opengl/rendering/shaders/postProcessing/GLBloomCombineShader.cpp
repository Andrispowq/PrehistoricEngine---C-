#include "Includes.hpp"
#include "GLBloomCombineShader.h"

namespace Prehistoric
{
	GLBloomCombineShader::GLBloomCombineShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/quad_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/bloom_combine_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		AddUniform("image0");
		AddUniform("image1");
	}

	void GLBloomCombineShader::UpdateUniforms(Texture* tex0, Texture* tex1) const
	{
		tex0->Bind(0);
		SetUniformi("image0", 0);
		tex1->Bind(1);
		SetUniformi("image1", 1);
	}
};