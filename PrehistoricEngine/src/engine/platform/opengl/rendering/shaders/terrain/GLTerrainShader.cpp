#include "Includes.hpp"
#include "GLTerrainShader.h"

#include "prehistoric/core/modules/terrain/TerrainNode.h"
#include "prehistoric/core/modules/terrain/Terrain.h"
#include "prehistoric/core/model/material/Material.h"

#include "prehistoric/core/config/EnvironmentMapConfig.h"

namespace Prehistoric
{
	GLTerrainShader::GLTerrainShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/terrain/terrain_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/terrain/terrain_TC.glsl"), TESSELLATION_CONTROL_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/terrain/terrain_TE.glsl"), TESSELLATION_EVALUATION_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/terrain/terrain_GS.glsl"), GEOMETRY_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/terrain/terrain_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		AddUniform("heightmap");

		location_localMatrix = glGetUniformLocation(program, "localMatrix");
		location_worldMatrix = glGetUniformLocation(program, "worldMatrix");

		AddUniform("cameraPosition");
		AddUniform("scaleY");

		location_location = glGetUniformLocation(program, "location");
		location_index = glGetUniformLocation(program, "index");
		location_gap = glGetUniformLocation(program, "gap");
		location_lod = glGetUniformLocation(program, "lod");

		for (unsigned int i = 0; i < 8; i++)
		{
			std::string uName = "lodMorphArea[" + std::to_string(i) + "]";

			AddUniform(uName);
		}

		AddUniform("tessellationFactor");
		AddUniform("tessellationSlope");
		AddUniform("tessellationShift");

		AddUniform("viewProjection");

		AddUniform("normalmap");
		AddUniform("splatmap");

		for (unsigned int i = 0; i < 3; i++)
		{
			std::string uniformName = "materials[" + std::to_string(i) + "].";

			AddUniform(uniformName + ALBEDO_MAP);
			AddUniform(uniformName + NORMAL_MAP);
			AddUniform(uniformName + MROT_MAP);

			AddUniform(uniformName + HEIGHT_SCALE);
			AddUniform(uniformName + HORIZONTAL_SCALE);
			AddUniform(uniformName + MROT);
		}

		AddUniform("irradianceMap");
		AddUniform("prefilterMap");
		AddUniform("brdfLUT");

		AddUniform("highDetailRange");
		AddUniform("numberOfTilesX");
		AddUniform("max_reflection_lod");
	}

	void GLTerrainShader::UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const
	{
		SetUniform("viewProjection", camera->getViewProjectionMatrix());
		SetUniform("cameraPosition", camera->getPosition());

		SetUniformi("highDetailRange", EngineConfig::rendererHighDetailRange);

		EnvironmentMapConfig::irradianceMap->Bind(0);
		SetUniformi("irradianceMap", 0);
		EnvironmentMapConfig::prefilterMap->Bind(1);
		SetUniformi("prefilterMap", 1);
		EnvironmentMapConfig::brdfLUT->Bind(2);
		SetUniformi("brdfLUT", 2);

		//TODO: This is ugly!!!! 
		SetUniformi("numberOfTilesX", FrameworkConfig::windowWidth / 16);
		SetUniformf("max_reflection_lod", EnvironmentMapConfig::prefilterLevels - 1.0f);

		//Some other stuff that is terrain-related
		SetUniformf("scaleY", TerrainConfig::scaleY);

		SetUniformi("tessellationFactor", TerrainConfig::tessellationFactor);
		SetUniformf("tessellationSlope", TerrainConfig::tessellationSlope);
		SetUniformf("tessellationShift", TerrainConfig::tessellationShift);

		for (unsigned int i = 0; i < 8; i++)
		{
			std::string uName = "lodMorphArea[" + std::to_string(i) + "]";

			SetUniformi(uName, TerrainConfig::lodMorphingAreas[i]);
		}
	}

	void GLTerrainShader::UpdateMaterialUniforms(Material* material, uint32_t descriptor_index) const
	{
		TerrainConfig::heightmap->Bind(3);
		SetUniformi("heightmap", 3);
		TerrainConfig::normalmap->Bind(4);
		SetUniformi("normalmap", 4);
		TerrainConfig::splatmap->Bind(5);
		SetUniformi("splatmap", 5);

		uint32_t texUnit = 6;

		for (unsigned int i = 0; i < 3; i++)
		{
			std::string uniformName = "materials[" + std::to_string(i) + "].";

			Material* material = TerrainConfig::terrainMaterials[i];

			material->getTexture(ALBEDO_MAP)->Bind(texUnit);
			SetUniformi(uniformName + ALBEDO_MAP, texUnit);
			texUnit++;

			material->getTexture(NORMAL_MAP)->Bind(texUnit);
			SetUniformi(uniformName + NORMAL_MAP, texUnit);
			texUnit++;

			material->getTexture(MROT_MAP)->Bind(texUnit);
			SetUniformi(uniformName + MROT_MAP, texUnit);
			texUnit++;

			SetUniformf(uniformName + HEIGHT_SCALE, material->getFloat(HEIGHT_SCALE));
			SetUniformf(uniformName + HORIZONTAL_SCALE, material->getFloat(HORIZONTAL_SCALE));
			SetUniform(uniformName + MROT, material->getVector4f(MROT));
		}
	}

	void GLTerrainShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		TerrainNode* node = (TerrainNode*)object;

		SetUniform(location_localMatrix, node->getLocalTransform().getTransformationMatrix());
		SetUniform(location_worldMatrix, node->getWorldTransform().getTransformationMatrix());

		SetUniform(location_location, node->getLocation());
		SetUniform(location_index, node->getIndex());
		SetUniformf(location_gap, node->getGap());
		SetUniformi(location_lod, node->getLod());
	}
};
