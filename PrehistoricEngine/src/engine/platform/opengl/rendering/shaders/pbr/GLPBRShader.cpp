#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLPBRShader.h"

#include "engine/prehistoric/core/config/EnvironmentMapConfig.h"

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

	for (uint32_t i = 0; i < EngineConfig::lightsMaxNumber; i++)
	{
		std::string name = "lights[" + std::to_string(i);

		AddUniform(name + "].position");
		AddUniform(name + "].colour");
		AddUniform(name + "].intensity");
	}

	AddUniform("irradianceMap");
	AddUniform("prefilterMap");
	AddUniform("brdfLUT");

	AddUniform("gamma");
	AddUniform("exposure");
	AddUniform("highDetailRange");
	AddUniform("numberOfLights");
}

void GLPBRShader::UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index) const
{
	SetUniform("m_view", camera->getViewMatrix());
	SetUniform("m_projection", camera->getProjectionMatrix());
	SetUniform("cameraPosition", camera->getPosition());

	uint32_t nOfLights = EngineConfig::lightsMaxNumber > (uint32_t)lights.size() ? (uint32_t)lights.size() : EngineConfig::lightsMaxNumber;
	SetUniformi("numberOfLights", nOfLights);
#if defined(PR_DEBUG)
	for (unsigned int i = 0; i < EngineConfig::lightsMaxNumber; i++)
	{
		std::string uniformName = "lights[" + std::to_string(i) + "].";

		if (i < lights.size())
		{
			Light* light = lights[i];

			SetUniform(uniformName + "position", light->getParent()->getWorldTransform().getPosition());
			SetUniform(uniformName + "colour", light->getColour());
			SetUniform(uniformName + "intensity", light->getIntensity());
		}
		else
		{
			//Load some dummy values for debug mode so we don't access undefined memory while debugging, but we won't in release mode
			SetUniform(uniformName + "position", Vector3f());
			SetUniform(uniformName + "colour", Vector3f());
			SetUniform(uniformName + "intensity", Vector3f());
		}
	}
#else
	for (unsigned int i = 0; i < nOfLights; i++)
	{
		std::string uniformName = "lights[" + std::to_string(i) + "].";

		Light* light = lights[i];

		SetUniform(uniformName + "position", light->getParent()->getWorldTransform().getPosition());
		SetUniform(uniformName + "colour", light->getColour());
		SetUniform(uniformName + "intensity", light->getIntensity());
	}
#endif

	EnvironmentMapConfig::irradianceMap->Bind(7);
	EnvironmentMapConfig::prefilterMap->Bind(8);
	EnvironmentMapConfig::brdfLUT->Bind(9);

	SetUniformi("irradianceMap", 7);
	SetUniformi("prefilterMap", 8);
	SetUniformi("brdfLUT", 9);

	SetUniformf("gamma", EngineConfig::rendererGamma);
	SetUniformf("exposure", EngineConfig::rendererExposure);
	SetUniformi("highDetailRange", EngineConfig::rendererHighDetailRange);
}

void GLPBRShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
{
	SetUniform(m_transform, object->getWorldTransform().getTransformationMatrix());

	Material* material = dynamic_cast<RendererComponent*>(object->GetComponent(RENDERER_COMPONENT))->getMaterial();

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