#include "Includes.hpp"
#include "GLGaussianShader.h"

namespace Prehistoric
{
	GLGaussianShader::GLGaussianShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/guassian_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("scene");
		AddUniform("screenSize");

		AddUniform("horizontal");
	}

	void GLGaussianShader::UpdateUniforms(Texture* scene, bool horizontal) const
	{
		scene->Bind();
		SetUniformi("scene", 0);

		SetUniformi("horizontal", horizontal);
		SetUniform("screenSize", Vector2f((float)FrameworkConfig::windowWidth, (float)FrameworkConfig::windowHeight));
	}
};