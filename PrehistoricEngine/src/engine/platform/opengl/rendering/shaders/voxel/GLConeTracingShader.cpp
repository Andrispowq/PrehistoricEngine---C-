#include "Includes.hpp"
#include "GLConeTracingShader.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	GLConeTracingShader::GLConeTracingShader() : GLShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxel_cone_tracing_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxel_cone_tracing_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		m_transform = glGetUniformLocation(program, "m_transform");
		AddUniform("m_view");
		AddUniform("m_projection");

		AddUniform("cameraPosition");
		/*AddUniform("highDetailRange");

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
		AddUniform("max_reflection_lod");*/


		AddUniform("material.diffuseColour");
		AddUniform("material.specularColour");
		AddUniform("material.diffuseReflectivity");
		AddUniform("material.specularReflectivity");
		AddUniform("material.emissivity");
		AddUniform("material.transparency");

		AddUniform("settings.indirectSpecularLight");
		AddUniform("settings.indirectDiffuseLight");
		AddUniform("settings.directLight");
		AddUniform("settings.shadows");

		AddUniform("pointLights[0].position");
		AddUniform("pointLights[0].colour");
		AddUniform("pointLights[0].intensity");

		AddUniform("numberOfLights");
		AddUniform("state");
		AddUniform("texture3D");
	}

	void GLConeTracingShader::UpdateGlobalUniforms(Renderer* renderer) const
	{
		Camera* camera = renderer->getCamera();
		const std::vector<Light*>& lights = renderer->getLights();

		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());
		SetUniform("cameraPosition", camera->getPosition());
		/*SetUniformi("highDetailRange", __EngineConfig.rendererHighDetailRange);

		__EnvironmentMapConfig.irradianceMap->Bind(4);
		SetUniformi("irradianceMap", 4);
		__EnvironmentMapConfig.prefilterMap->Bind(5);
		SetUniformi("prefilterMap", 5);
		__EnvironmentMapConfig.brdfLUT->Bind(6);
		SetUniformi("brdfLUT", 6);

		SetUniformi("numberOfTilesX", __FrameworkConfig.windowWidth / 16);
		SetUniformf("max_reflection_lod", __EnvironmentMapConfig.prefilterLevels - 1.0f);*/

		SetUniformi("settings.indirectSpecularLight", 1);
		SetUniformi("settings.indirectDiffuseLight", 1);
		SetUniformi("settings.directLight", 1);
		SetUniformi("settings.shadows", 1);

		SetUniform("pointLights[0].position", Vector3f(0, 50, 0));
		SetUniform("pointLights[0].colour", Vector3f(1, 1, 1));
		SetUniformf("pointLights[0].intensity", 4000);

		SetUniformi("numberOfLights", 1);
		SetUniformi("state", 0);
		SetUniformi("texture3D", 0); //It will be bound as tex num 0
	}

	void GLConeTracingShader::UpdateMaterialUniforms(Material* material, uint32_t descriptor_index) const
	{
		/*material->getTexture(ALBEDO_MAP)->Bind(0);
		SetUniformi(albedoMap, 0);
		material->getTexture(NORMAL_MAP)->Bind(1);
		SetUniformi(normalMap, 1);
		material->getTexture(MROT_MAP)->Bind(2);
		SetUniformi(mrotMap, 2);

		SetUniform(colour, material->getVector3f(COLOUR));
		SetUniformi(usesNormalMap, material->exists(NORMAL_MAP));
		SetUniform(mrot, material->getVector4f(MROT));
		SetUniformf(emission, material->getFloat(EMISSION));*/

		Vector3f diffuse = material->getVector3f(COLOUR);
		if (diffuse == -1)
		{
			diffuse = Vector3f(1, 0, 0);
		}

		Vector4f mrot = material->getVector4f(MROT);
		if (mrot == -1)
		{
			mrot = Vector4f(0.5f, 0.5f, 0.0f, 1.0f);
		}

		SetUniform("material.diffuseColour", diffuse);
		SetUniform("material.specularColour", diffuse);
		SetUniformf("material.diffuseReflectivity", mrot.r);
		SetUniformf("material.specularReflectivity", mrot.g);
		SetUniformf("material.emissivity", material->getFloat(EMISSION));
		SetUniformf("material.transparency", 0.0f);
	}

	void GLConeTracingShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		SetUniform(m_transform, object->getWorldTransform().getTransformationMatrix());
	}
}