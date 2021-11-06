#include "Includes.hpp"
#include "GLBloomDecomposeShader.h"

namespace Prehistoric
{
	GLBloomDecomposeShader::GLBloomDecomposeShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/bloom_decompose_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("scene");
		AddUniform("threshold");
		AddUniform("targetSize");
	}

	void GLBloomDecomposeShader::UpdateUniforms(Texture* scene, float threshold, Vector2f targetSize) const
	{
		scene->Bind();
		SetUniformi("scene", 0);

		SetUniformf("threshold", threshold);
		SetUniform("targetSize", targetSize);
	}
};