#include "Includes.hpp"
#include "GLFXAAShader.h"

#include "platform/opengl/rendering/GLRenderer.h"

namespace Prehistoric
{
	GLFXAAShader::GLFXAAShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/deferred/fxaa_CS.glsl"), COMPUTE_SHADER);
		//AddShader(ResourceLoader::LoadShaderGL("opengl/deferred/deferred_VS.glsl"), VERTEX_SHADER);
		//AddShader(ResourceLoader::LoadShaderGL("opengl/deferred/deferred_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		AddUniform("sceneSampler");
		AddUniform("dimension");
	}

	void GLFXAAShader::UpdateUniforms(Renderer* renderer, Camera* camera, const std::vector<Light*>& lights) const
	{
		GLRenderer* rend = (GLRenderer*)renderer;
		rend->getOutputTexture()->Bind();
		SetUniformi("sceneSampler", 0);

		Window * window = renderer->getWindow();
		SetUniform("dimension", Vector2f((float)window->getWidth(), (float)window->getHeight()));
	}
};
