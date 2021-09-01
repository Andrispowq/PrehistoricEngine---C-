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

	void GLAtmosphereShader::UpdateUniforms(Matrix4f view, Matrix4f projection) const
	{
		SetUniform("m_transform", Matrix4f::Identity());
		SetUniform("m_view", view);
		SetUniform("m_projection", projection);
	}

	void GLAtmosphereShader::UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const
	{
		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());
	}

	void GLAtmosphereShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix());
	}
};
