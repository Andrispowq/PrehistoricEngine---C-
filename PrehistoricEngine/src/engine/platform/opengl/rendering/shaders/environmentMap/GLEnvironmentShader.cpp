#include "Includes.hpp"
#include "GLEnvironmentShader.h"

namespace Prehistoric
{
	GLEnvironmentShader::GLEnvironmentShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/environment_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/environment_FS.glsl"), FRAGMENT_SHADER);

		CompileShader();

		AddUniform("m_transform");
		AddUniform("m_view");
		AddUniform("m_projection");

		AddUniform("environmentMap");
		AddUniform("gamma");
		AddUniform("exposure");
	}

	void GLEnvironmentShader::UpdateUniforms(const Matrix4f& projection, const Matrix4f& view, Texture* texture) const
	{
		SetUniform("m_transform", Matrix4f::Transformation(Vector3f(0, 10, 0), Vector3f(), Vector3f(1)));
		SetUniform("m_view", view);
		SetUniform("m_projection", projection);

		texture->Bind(0);
		SetUniformi("environmentMap", 0);

		SetUniformf("gamma", EngineConfig::rendererGamma);
		SetUniformf("exposure", EngineConfig::rendererExposure);
	}
};
