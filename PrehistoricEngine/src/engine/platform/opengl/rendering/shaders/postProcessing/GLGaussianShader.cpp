#include "Includes.hpp"
#include "GLGaussianShader.h"

namespace Prehistoric
{
	GLGaussianShader::GLGaussianShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/quad_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/gaussian_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		AddUniform("scene");
		AddUniform("horizontal");

		AddUniform("targetSize");
	}

	void GLGaussianShader::UpdateUniforms(Texture* scene, bool horizontal, Vector2f targetSize) const
	{
		scene->Bind();
		SetUniformi("scene", 0);

		SetUniformi("horizontal", horizontal);
		SetUniform("targetSize", targetSize);
	}
};