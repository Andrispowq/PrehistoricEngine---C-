#include "Includes.hpp"
#include "GLBLoomDescaleShader.h"

namespace Prehistoric
{
	GLBloomDescaleShader::GLBloomDescaleShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/bloom_descale_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/bloom_descale_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		AddUniform("scene");
	}

	void GLBloomDescaleShader::UpdateUniforms(Texture* scene) const
	{
		scene->Bind();
		SetUniformi("scene", 0);
	}
};