#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLIrradianceShader.h"

namespace Prehistoric
{
	GLIrradianceShader::GLIrradianceShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/irradiance_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/environmentMap/irradiance_FS.glsl"), FRAGMENT_SHADER);

		CompileShader();

		AddUniform("m_view");
		AddUniform("m_projection");

		AddUniform("environmentMap");
	}

	void GLIrradianceShader::UpdateUniforms(const Matrix4f& projection, const Matrix4f& view, Texture* texture) const
	{
		SetUniform("m_view", view);
		SetUniform("m_projection", projection);

		texture->Bind(0);
		SetUniformi("environmentMap", 0);
	}
};
