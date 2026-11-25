#include "Includes.hpp"
#include "GLPBRShader.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "prehistoric/application/Application.h"

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

		AddUniform("captureLightCount");
		AddUniform("numberOfTilesX");
		AddUniform("max_reflection_lod");

		AddShadowUniforms();
		AddUniform("useVoxelPointShadows");
		AddUniform("time");
	}

	void GLPBRShader::UpdateGlobalUniforms(Renderer* renderer) const
	{
		Camera* camera = renderer->getCamera();
		const auto* culling = static_cast<GLRenderer*>(renderer)->getLightCullingPass();

		SetUniform("m_view", camera->getViewMatrix());
		SetUniform("m_projection", camera->getProjectionMatrix());
		SetUniform("cameraPosition", camera->getPosition());
		SetUniformi("highDetailRange", __EngineConfig.rendererHighDetailRange);

		BindEnvironmentLighting(renderer, 4);

		SetUniformi("captureLightCount", __WaterConfig.stage == WaterRenderStage::Default ? -1 : culling->getUploadedLightCount());
		SetUniformi("numberOfTilesX", GLLightCullingPass::TileCount(renderer->getWindow()->getWidth()));
		SetUniformf("max_reflection_lod", __EnvironmentMapConfig.prefilterLevels - 1.0f);

		BindShadowUniforms(renderer, 3);
        auto* voxels = static_cast<GLRenderer*>(renderer)->getVoxelPass();
        SetUniformi("useVoxelPointShadows", voxels->HasOccluders()
            && voxels->GetSettings().globalIllumination);
		SetUniformf("time", (float)Time::getTimeFromStart());
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