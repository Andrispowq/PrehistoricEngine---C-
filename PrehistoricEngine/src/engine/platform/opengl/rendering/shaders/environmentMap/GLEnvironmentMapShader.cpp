#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLEnvironmentMapShader.h"

namespace Prehistoric
{
	GLEnvironmentMapShader::GLEnvironmentMapShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/environmentMap_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/environmentMap_FS.glsl"), FRAGMENT_SHADER);

		CompileShader();

		AddUniform("m_view");
		AddUniform("m_projection");

		AddUniform("equirectangularMap");
	}

	void GLEnvironmentMapShader::UpdateUniforms(const Matrix4f& projection, const Matrix4f& view, Texture* texture) const
	{
		SetUniform("m_view", view);
		SetUniform("m_projection", projection);

		texture->Bind(0);
		SetUniformi("equirectangularMap", 0);
	}
};
