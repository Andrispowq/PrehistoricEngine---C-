#include "Includes.hpp"
#include "GLBloomDecomposeShader.h"

namespace Prehistoric
{
	GLBloomDecomposeShader::GLBloomDecomposeShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/quad_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/bloom_decompose_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		AddUniform("scene");
		AddUniform("threshold");
	}

	void GLBloomDecomposeShader::UpdateUniforms(Texture* scene, float threshold) const
	{
		scene->Bind();
		SetUniformi("scene", 0);

		SetUniformf("threshold", threshold);
	}
};