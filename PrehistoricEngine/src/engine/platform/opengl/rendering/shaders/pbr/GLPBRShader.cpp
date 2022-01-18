#include "Includes.hpp"
#include "GLPBRShader.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"
#include "prehistoric/common/buffer/UniformBufferObject.h"

namespace Prehistoric
{
	extern UniformBufferObject* _matrices;
	extern Texture* _shadowMap;
	extern std::vector<float> cascadeDistances;

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

		albedoMap = glGetUniformLocation(program, (std::string("material.") + ALBEDO_MAP).c_str());
		normalMap = glGetUniformLocation(program, (std::string("material.") + NORMAL_MAP).c_str());
		mrotMap = glGetUniformLocation(program, (std::string("material.") + MROT_MAP).c_str());

		colour = glGetUniformLocation(program, (std::string("material.") + COLOUR).c_str());
		usesNormalMap = glGetUniformLocation(program, (std::string("material.") + USES_NORMAL_MAP).c_str());
		mrot = glGetUniformLocation(program, (std::string("material.") + MROT).c_str());
		emission = glGetUniformLocation(program, (std::string("material.") + EMISSION).c_str());

		AddUniform("irradianceMap");
		AddUniform("prefilterMap");
		AddUniform("brdfLUT");

		AddUniform("numberOfTilesX");
		AddUniform("max_reflection_lod");

		AddUniformBlock("LightSpaceMatrices");
		AddUniform("cascadeCount");
		AddUniform("farPlane");
		AddUniform("shadowMap");

		AddUniform("time");

		for(uint32_t i = 0; i < cascadeDistances.size(); i++)
			AddUniform("cascadePlaneDistances[" + std::to_string(i) + "]");
	}

	void GLPBRShader::UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const
	{
		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());
		SetUniform("cameraPosition", camera->getPosition());
		SetUniformi("highDetailRange", EngineConfig::rendererHighDetailRange);

		_matrices->BindBase(nullptr, 0);
		SetUniformBlock("LightSpaceMatrices", 0);
		SetUniformi("cascadeCount", cascadeDistances.size());
		SetUniformf("farPlane", EngineConfig::rendererFarPlane);
		_shadowMap->Bind(3);
		SetUniformi("shadowMap", 3);

		SetUniformf("time", Time::getTimeFromStart());

		for (uint32_t i = 0; i < cascadeDistances.size(); i++)
			SetUniformf("cascadePlaneDistances[" + std::to_string(i) + "]", cascadeDistances[i]);

		EnvironmentMapConfig::irradianceMap->Bind(4);
		SetUniformi("irradianceMap", 4);
		EnvironmentMapConfig::prefilterMap->Bind(5);
		SetUniformi("prefilterMap", 5);
		EnvironmentMapConfig::brdfLUT->Bind(6);
		SetUniformi("brdfLUT", 6);

		//TODO: This is ugly!!!! 
		SetUniformi("numberOfTilesX", FrameworkConfig::windowWidth / 16);
		SetUniformf("max_reflection_lod", EnvironmentMapConfig::prefilterLevels - 1.0f);
	}

	void GLPBRShader::UpdateMaterialUniforms(Material* material, uint32_t descriptor_index) const
	{
		material->getTexture(ALBEDO_MAP)->Bind(0);
		SetUniformi(albedoMap, 0);
		material->getTexture(NORMAL_MAP)->Bind(1);
		SetUniformi(normalMap, 1);
		material->getTexture(MROT_MAP)->Bind(2);
		SetUniformi(mrotMap, 2);

		SetUniform(colour, material->getVector3f(COLOUR));
		SetUniformi(usesNormalMap, material->exists(NORMAL_MAP));
		SetUniform(mrot, material->getVector4f(MROT));
		SetUniformf(emission, material->getFloat(EMISSION));
	}

	void GLPBRShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		SetUniform(m_transform, object->getWorldTransform().getTransformationMatrix());
	}
};