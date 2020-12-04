#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLTerrainShader.h"

#include "engine/prehistoric/modules/terrain/TerrainNode.h"
#include "engine/prehistoric/core/model/material/Material.h"

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
		AddUniform(uniformName + DISPLACEMENT_MAP);
		AddUniform(uniformName + METALLIC_MAP);
		AddUniform(uniformName + ROUGHNESS_MAP);
		//AddUniform(uniformName + OCCLUSION_MAP);

		AddUniform(uniformName + HEIGHT_SCALE);
		AddUniform(uniformName + HORIZONTAL_SCALE);
		AddUniform(uniformName + METALLIC);
		AddUniform(uniformName + ROUGHNESS);
		//AddUniform(uniformName. + OCCLUSION);
	}

	for (unsigned int i = 0; i < EngineConfig::lightsMaxNumber; i++)
	{
		std::string uniformName = "lights[" + std::to_string(i) + "].";

		AddUniform(uniformName + "position");
		AddUniform(uniformName + "colour");
		AddUniform(uniformName + "intensity");
	}

	AddUniform("highDetailRange");
	AddUniform("gamma");
	AddUniform("exposure");
	AddUniform("numberOfLights");

	//AddUniform("irradianceMap");
	//AddUniform("prefilterMap");
	//AddUniform("brdfLUT");
}

void GLTerrainShader::UpdateShaderUniforms(Camera* camera, const std::vector<Light*>& lights, uint32_t instance_index) const
{
	SetUniform("viewProjection", camera->getViewProjectionMatrix());
	SetUniform("cameraPosition", camera->getPosition());

	uint32_t nOfLights = (uint32_t)lights.size() < EngineConfig::lightsMaxNumber ? (uint32_t)lights.size() : EngineConfig::lightsMaxNumber;

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

	SetUniformi("highDetailRange", EngineConfig::rendererHighDetailRange);
	SetUniformf("gamma", EngineConfig::rendererGamma);
	SetUniformf("exposure", EngineConfig::rendererExposure);
	SetUniformi("numberOfLights", nOfLights);

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

	uint32_t texUnit = 3;

	for (unsigned int i = 0; i < 3; i++)
	{
		std::string uniformName = "materials[" + std::to_string(i) + "].";

		Material* material = TerrainConfig::materials[i];

		material->getTexture(ALBEDO_MAP)->Bind(texUnit);
		SetUniformi(uniformName + ALBEDO_MAP, texUnit);
		texUnit++;

		material->getTexture(NORMAL_MAP)->Bind(texUnit);
		SetUniformi(uniformName + NORMAL_MAP, texUnit);
		texUnit++;

		material->getTexture(DISPLACEMENT_MAP)->Bind(texUnit);
		SetUniformi(uniformName + DISPLACEMENT_MAP, texUnit);
		texUnit++;

		material->getTexture(METALLIC_MAP)->Bind(texUnit);
		SetUniformi(uniformName + METALLIC_MAP, texUnit);
		texUnit++;

		material->getTexture(ROUGHNESS_MAP)->Bind(texUnit);
		SetUniformi(uniformName + ROUGHNESS_MAP, texUnit);
		texUnit++;

		SetUniformf(uniformName + HEIGHT_SCALE, material->getFloat(HEIGHT_SCALE));
		SetUniformf(uniformName + HORIZONTAL_SCALE, material->getFloat(HORIZONTAL_SCALE));
		SetUniformf(uniformName + METALLIC, material->getFloat(METALLIC));
		SetUniformf(uniformName + ROUGHNESS, material->getFloat(ROUGHNESS));
		//SetUniformf(uniformName + OCCLUSION, material->getFloat(OCCLUSION));
	}
}

void GLTerrainShader::UpdateSharedUniforms(GameObject* object, uint32_t instance_index) const
{
	TerrainNode* node = (TerrainNode*)object;

	node->getMaps()->getHeightmap()->Bind(0);
	SetUniformi("heightmap", 0);
	node->getMaps()->getNormalmap()->Bind(1);
	SetUniformi("normalmap", 1);
	node->getMaps()->getSplatmap()->Bind(2);
	SetUniformi("splatmap", 2);
}

void GLTerrainShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
{
	TerrainNode* node = (TerrainNode*)object;

	SetUniform(location_localMatrix, node->getLocalTransform().getTransformationMatrix());
	SetUniform(location_worldMatrix, object->getWorldTransform().getTransformationMatrix());

	SetUniform(location_location, node->getLocation());
	SetUniform(location_index, node->getIndex());
	SetUniformf(location_gap, node->getGap());
	SetUniformi(location_lod, node->getLod());
}
