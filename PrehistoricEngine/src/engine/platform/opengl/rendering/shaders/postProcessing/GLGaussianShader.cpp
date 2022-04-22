#include "Includes.hpp"
#include "GLGaussianShader.h"

namespace Prehistoric
{
	GLGaussianShader::GLGaussianShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/gaussian_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("scene");
		AddUniform("horizontal");

		AddUniform("targetSize");
		AddUniform("sceneSize");
	}

	void GLGaussianShader::UpdateUniforms(Texture* scene, bool horizontal, Vector2f targetSize, Vector2f sceneSize) const
	{
		scene->Bind();
		SetUniformi("scene", 0);

		SetUniformi("horizontal", horizontal);
		SetUniform("targetSize", targetSize);
		SetUniform("sceneSize", sceneSize);
	}
};