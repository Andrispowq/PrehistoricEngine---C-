#include "Includes.hpp"
#include "GLBasicShader.h"

namespace Prehistoric
{
	GLBasicShader::GLBasicShader() : GLShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/basic/basic_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/basic/basic_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		m_transform = glGetUniformLocation(program, "m_transform");
		AddUniform("m_view");
		AddUniform("m_projection");

		AddUniform("cameraPosition");

		albedoMap = glGetUniformLocation(program, (std::string("material.") + ALBEDO_MAP).c_str());
		mrotMap = glGetUniformLocation(program, (std::string("material.") + MROT_MAP).c_str());

		colour = glGetUniformLocation(program, (std::string("material.") + COLOUR).c_str());
		mrot = glGetUniformLocation(program, (std::string("material.") + MROT).c_str());
	}

	void GLBasicShader::UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const
	{
		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());
		SetUniform("cameraPosition", camera->getPosition());
	}

	void GLBasicShader::UpdateTextureUniforms(Material* material, uint32_t descriptor_index) const
	{
		material->getTexture(ALBEDO_MAP)->Bind(0);
		SetUniformi(albedoMap, 0);
		material->getTexture(MROT_MAP)->Bind(1);
		SetUniformi(mrotMap, 1);
	}

	void GLBasicShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		SetUniform(m_transform, object->getWorldTransform().getTransformationMatrix());

		Material* material = object->GetComponent<RendererComponent>()->getMaterial();

		SetUniform(colour, material->getVector3f(COLOUR));
		SetUniform(mrot, material->getVector4f(MROT));
	}
};
