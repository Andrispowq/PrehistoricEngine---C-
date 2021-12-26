#include "Includes.hpp"
#include "GLDepthPassShader.h"

namespace Prehistoric
{
	GLDepthPassShader::GLDepthPassShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/forwardPlus/depth_pass_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/forwardPlus/depth_pass_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		m_transform = glGetUniformLocation(program, "m_transform");
		AddUniform("m_view");
		AddUniform("m_projection");
	}

	void GLDepthPassShader::UpdateGlobalUniforms(Matrix4f proj, Matrix4f view)
	{
		SetUniform("m_view", view);
		SetUniform("m_projection", proj);
	}

	void GLDepthPassShader::UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const
	{
		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());
	}

	void GLDepthPassShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		SetUniform(m_transform, object->getWorldTransform().getTransformationMatrix());
	}
};

