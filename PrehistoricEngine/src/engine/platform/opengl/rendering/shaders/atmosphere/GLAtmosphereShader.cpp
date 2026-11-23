#include "Includes.hpp"
#include "GLAtmosphereShader.h"

#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric
{
	GLAtmosphereShader::GLAtmosphereShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/atmosphere/atmosphere_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/atmosphere/atmosphere_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		AddUniform("m_transform");
		AddUniform("m_view");
		AddUniform("m_projection");
	}

	void GLAtmosphereShader::UpdateUniforms(const Matrix4f& transform, const Matrix4f& view, const Matrix4f& projection) const
	{
		SetUniform("m_transform", transform);
		SetUniform("m_view", view);
		SetUniform("m_projection", projection);
	}

}
