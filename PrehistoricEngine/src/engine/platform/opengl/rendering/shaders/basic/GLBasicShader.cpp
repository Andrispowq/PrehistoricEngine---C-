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

		albedoMap = glGetUniformLocation(program, "material.albedoMap");
		metallicMap = glGetUniformLocation(program, "material.metallicMap");
		roughnessMap = glGetUniformLocation(program, "material.roughnessMap");

		colour = glGetUniformLocation(program, "material.colour");
		metallic = glGetUniformLocation(program, "material.metallic");
		roughness = glGetUniformLocation(program, "material.roughness");
	}

	void GLBasicShader::UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index) const
	{
		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());
		SetUniform("cameraPosition", camera->getPosition());
	}

	void GLBasicShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		SetUniform(m_transform, object->getWorldTransform().getTransformationMatrix());

		Material* material = dynamic_cast<RendererComponent*>(object->GetComponent("RendererComponent"))->getMaterial();

		material->getTexture("albedoMap")->Bind(0);
		SetUniformi(albedoMap, 0);
		material->getTexture("metallicMap")->Bind(1);
		SetUniformi(metallicMap, 2);
		material->getTexture("roughnessMap")->Bind(2);
		SetUniformi(roughnessMap, 2);

		SetUniform(colour, material->getVector3f("colour"));
		SetUniformf(metallic, material->getFloat("metallic"));
		SetUniformf(roughness, material->getFloat("roughness"));
	}
};
