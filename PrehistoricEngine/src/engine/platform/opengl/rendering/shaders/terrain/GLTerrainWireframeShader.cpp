#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLTerrainWireframeShader.h"

#include "engine/prehistoric/core/modules/terrain/TerrainNode.h"
#include "engine/prehistoric/core/model/material/Material.h"

namespace Prehistoric
{
	GLTerrainWireframeShader::GLTerrainWireframeShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/terrain/terrain_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/terrain/terrain_TC.glsl"), TESSELLATION_CONTROL_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/terrain/terrain_TE.glsl"), TESSELLATION_EVALUATION_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/terrain/terrain_wGS.glsl"), GEOMETRY_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/terrain/terrain_wFS.glsl"), FRAGMENT_SHADER);
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

		AddUniform("splatmap");

		for (unsigned int i = 0; i < 3; i++)
		{
			std::string uniformName = "materials[" + std::to_string(i) + "].";

			AddUniform(uniformName + DISPLACEMENT_MAP);

			AddUniform(uniformName + HEIGHT_SCALE);
			AddUniform(uniformName + HORIZONTAL_SCALE);
		}

		AddUniform("highDetailRange");
	}

	void GLTerrainWireframeShader::UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index) const
	{
		SetUniform("viewProjection", camera->getViewProjectionMatrix());
		SetUniform("cameraPosition", camera->getPosition());

		SetUniformf("scaleY", TerrainConfig::scaleY);

		SetUniformi("tessellationFactor", TerrainConfig::tessellationFactor);
		SetUniformf("tessellationSlope", TerrainConfig::tessellationSlope);
		SetUniformf("tessellationShift", TerrainConfig::tessellationShift);

		uint32_t texUnit = 3;

		for (unsigned int i = 0; i < 3; i++)
		{
			std::string uniformName = "materials[" + std::to_string(i) + "].";

			Material* material = TerrainConfig::materials[i];

			material->getTexture(DISPLACEMENT_MAP)->Bind(texUnit);
			SetUniformi(uniformName + DISPLACEMENT_MAP, texUnit);
			texUnit++;

			SetUniformf(uniformName + HEIGHT_SCALE, material->getFloat(HEIGHT_SCALE));
			SetUniformf(uniformName + HORIZONTAL_SCALE, material->getFloat(HORIZONTAL_SCALE));
		}

		for (unsigned int i = 0; i < 8; i++)
		{
			std::string uName = "lodMorphArea[" + std::to_string(i) + "]";

			SetUniformi(uName, TerrainConfig::lodMorphingAreas[i]);
		}

		SetUniformi("highDetailRange", EngineConfig::rendererHighDetailRange);
	}

	void GLTerrainWireframeShader::UpdateSharedUniforms(GameObject* object, uint32_t instance_index) const
	{
		TerrainNode* node = (TerrainNode*)object;

		node->getMaps()->getHeightmap()->Bind(0);
		SetUniformi("heightmap", 0);
		node->getMaps()->getSplatmap()->Bind(1);
		SetUniformi("splatmap", 1);
	}

	void GLTerrainWireframeShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		TerrainNode* node = (TerrainNode*)object;

		SetUniform(location_localMatrix, node->getLocalTransform().getTransformationMatrix());
		SetUniform(location_worldMatrix, object->getWorldTransform().getTransformationMatrix());

		SetUniform(location_location, node->getLocation());
		SetUniform(location_index, node->getIndex());
		SetUniformf(location_gap, node->getGap());
		SetUniformi(location_lod, node->getLod());
	}
};
