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
		displacementMap = glGetUniformLocation(program, "material.displacementMap");
		roughnessMap = glGetUniformLocation(program, "material.roughnessMap");
		metallicMap = glGetUniformLocation(program, "material.metallicMap");
		occlusionMap = glGetUniformLocation(program, "material.occlusionMap");
		emissionMap = glGetUniformLocation(program, "material.emissionMap");

		colour = glGetUniformLocation(program, "material.colour");
		usesNormalMap = glGetUniformLocation(program, "material.usesNormalMap");
		heightScale = glGetUniformLocation(program, "material.heightScale");
		roughness = glGetUniformLocation(program, "material.roughness");
		metallic = glGetUniformLocation(program, "material.metallic");
		ambientOcclusion = glGetUniformLocation(program, "material.ambientOcclusion");
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
		material->getTexture("displacementMap")->Bind(2);
		SetUniformi(displacementMap, 2);
		material->getTexture("metallicMap")->Bind(3);
		SetUniformi(metallicMap, 3);
		material->getTexture("roughnessMap")->Bind(4);
		SetUniformi(roughnessMap, 4);
		material->getTexture("occlusionMap")->Bind(5);
		SetUniformi(occlusionMap, 5);
		material->getTexture("emissionMap")->Bind(6);
		SetUniformi(emissionMap, 6);

		SetUniform(colour, material->getVector3f("colour"));
		SetUniformi(usesNormalMap, material->exists("normalMap"));
		SetUniformf(heightScale, material->getFloat("heightScale"));
		SetUniformf(metallic, material->getFloat("metallic"));
		SetUniformf(roughness, material->getFloat("roughness"));
		SetUniformf(ambientOcclusion, material->getFloat("ambientOcclusion"));
		SetUniform(emission, material->getVector3f("emission"));
	}
};