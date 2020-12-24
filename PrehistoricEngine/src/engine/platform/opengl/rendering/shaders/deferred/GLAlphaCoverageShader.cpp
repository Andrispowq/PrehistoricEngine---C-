#include "Includes.hpp"
#include "GLAlphaCoverageShader.h"

#include "platform/opengl/rendering/GLRenderer.h"

namespace Prehistoric
{
	GLAlphaCoverageShader::GLAlphaCoverageShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/deferred/alphaCoverage_CS.glsl"), COMPUTE_SHADER);
		CompileShader();

		AddUniform("positionMetallic");
		AddUniform("dimension");
		AddUniform("sampleCount");
	}

	void GLAlphaCoverageShader::UpdateUniforms(Renderer* renderer, Camera* camera, const std::vector<Light*>& lights) const
	{
		GLRenderer* rend = (GLRenderer*)renderer;
		rend->getPositionMetallic()->Bind();
		SetUniformi("positionMetallic", 0);

		Window* window = renderer->getWindow();
		SetUniform("dimension", Vector2f((float)window->getWidth(), (float)window->getHeight()));
		SetUniformi("sampleCount", FrameworkConfig::windowNumSamples);
	}
}