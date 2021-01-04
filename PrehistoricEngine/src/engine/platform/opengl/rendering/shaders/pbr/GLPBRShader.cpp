#include "Includes.hpp"
#include "GLPBRShader.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"

namespace Prehistoric
{
	GLPBRShader::GLPBRShader() : GLShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/pbr/pbr_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/pbr/pbr_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		m_transform = glGetUniformLocation(program, "m_transform");
		AddUniform("m_view");
		AddUniform("m_projection");

		AddUniform("cameraPosition");
		AddUniform("highDetailRange");

		albedoMap = glGetUniformLocation(program, "material.albedoMap");
		normalMap = glGetUniformLocation(program, "material.normalMap");
		mrotMap = glGetUniformLocation(program, "material.mrotMap");
		emissionMap = glGetUniformLocation(program, "material.emissionMap");

		colour = glGetUniformLocation(program, "material.colour");
		usesNormalMap = glGetUniformLocation(program, "material.usesNormalMap");
		mrot = glGetUniformLocation(program, "material.mrot");
		emission = glGetUniformLocation(program, "material.emission");

		AddUniform("highDetailRange");
	}

	void GLPBRShader::UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index) const
	{
		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());
		SetUniform("cameraPosition", camera->getPosition());
		SetUniformi("highDetailRange", EngineConfig::rendererHighDetailRange);
	}

	void GLPBRShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		SetUniform(m_transform, object->getWorldTransform().getTransformationMatrix());

		Material* material = static_cast<RendererComponent*>(object->GetComponent(RENDERER_COMPONENT))->getMaterial();

		material->getTexture("albedoMap")->Bind(0);
		SetUniformi(albedoMap, 0);
		material->getTexture("normalMap")->Bind(1);
		SetUniformi(normalMap, 1);
		material->getTexture("mrotMap")->Bind(2);
		SetUniformi(mrotMap, 2);
		material->getTexture("emissionMap")->Bind(3);
		SetUniformi(emissionMap, 3);

		SetUniform(colour, material->getVector3f("colour"));
		SetUniformi(usesNormalMap, material->exists("normalMap"));
		SetUniform(mrot, material->getVector4f("mrot"));
		SetUniform(emission, material->getVector3f("emission"));
	}
};