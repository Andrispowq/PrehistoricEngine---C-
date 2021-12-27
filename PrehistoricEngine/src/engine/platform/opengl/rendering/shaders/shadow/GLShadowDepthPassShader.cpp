#include "Includes.hpp"
#include "GLShadowDepthPassShader.h"

namespace Prehistoric
{
	GLShadowDepthPassShader::GLShadowDepthPassShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/shadow/depth_pass_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/shadow/depth_pass_GS.glsl"), GEOMETRY_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/shadow/depth_pass_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		m_transform = glGetUniformLocation(program, "m_transform");
		AddUniformBlock("LightSpaceMatrices");
	}

	void GLShadowDepthPassShader::UpdateGlobalUniforms(UniformBufferObject* matrices)
	{
		matrices->BindBase(nullptr, 0);
		SetUniformBlock("LightSpaceMatrices", 0);
	}
};
