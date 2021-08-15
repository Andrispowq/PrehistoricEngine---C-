#include "Includes.hpp"
#include "GLEnvironmentShader.h"

namespace Prehistoric
{
	GLEnvironmentShader::GLEnvironmentShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/environment_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/environment_FS.glsl"), FRAGMENT_SHADER);

		CompileShader();

		AddUniform("m_view");
		AddUniform("m_projection");

		AddUniform("environmentMap");
		AddUniform("lod");
	}

	void GLEnvironmentShader::UpdateUniforms(const Matrix4f& projection, const Matrix4f& view, Texture* texture, float lod) const
	{
		SetUniform("m_view", view);
		SetUniform("m_projection", projection);

		texture->Bind(0);
		SetUniformi("environmentMap", 0);
		SetUniformf("lod", lod);
	}
};
