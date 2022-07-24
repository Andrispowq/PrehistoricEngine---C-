#include "Includes.hpp"
#include "GLWaterWireframeShader.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	GLWaterWireframeShader::GLWaterWireframeShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_TC.glsl"), TESSELLATION_CONTROL_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_TE.glsl"), TESSELLATION_EVALUATION_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_wGS.glsl"), GEOMETRY_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/water/water_wFS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		location_localMatrix = glGetUniformLocation(program, "localMatrix");
		location_worldMatrix = glGetUniformLocation(program, "worldMatrix");

		location_location = glGetUniformLocation(program, "location");
		location_index = glGetUniformLocation(program, "index");
		location_gap = glGetUniformLocation(program, "gap");
		location_lod = glGetUniformLocation(program, "lod");

		AddUniform("clipplane");
		AddUniform("scaleY");

		for (int i = 0; i < 8; i++)
		{
			std::string uName = "lodMorphArea[" + std::to_string(i) + "]";
			AddUniform(uName);
		}

		AddUniform("tessellationFactor");
		AddUniform("tessellationSlope");
		AddUniform("tessellationShift");

		AddUniform("tiling");

		AddUniform("Dy");
		AddUniform("Dx");
		AddUniform("Dz");

		AddUniform("viewProjection");

		AddUniform("cameraPosition");
		AddUniform("wind");

		AddUniform("displacementScale");
		AddUniform("choppiness");
		AddUniform("motion");

		AddUniform("displacementRange");
	}

	void GLWaterWireframeShader::UpdateGlobalUniforms(Renderer* renderer) const
	{
		Camera* camera = renderer->getCamera();
		const std::vector<Light*>& lights = renderer->getLights();

		SetUniform("viewProjection", camera->getViewProjectionMatrix());
		SetUniform("cameraPosition", camera->getPosition());

		//SetUniformi("highDetailRange", __EngineConfig.rendererHighDetailRange);

		/*__EnvironmentMapConfig.irradianceMap->Bind(0);
		SetUniformi("irradianceMap", 0);
		__EnvironmentMapConfig.prefilterMap->Bind(1);
		SetUniformi("prefilterMap", 1);
		__EnvironmentMapConfig.brdfLUT->Bind(2);
		SetUniformi("brdfLUT", 2);

		//TODO: This is ugly!!!!
		SetUniformi("numberOfTilesX", __FrameworkConfig.windowWidth / 16);
		SetUniformf("max_reflection_lod", __EnvironmentMapConfig.prefilterLevels - 1.0f);*/

		//Some other stuff that is terrain-related
		SetUniformf("scaleY", __TerrainConfig.scaleY);

		SetUniformi("tessellationFactor", __TerrainConfig.tessellationFactor);
		SetUniformf("tessellationSlope", __TerrainConfig.tessellationSlope);
		SetUniformf("tessellationShift", __TerrainConfig.tessellationShift);

		for (unsigned int i = 0; i < 8; i++)
		{
			std::string uName = "lodMorphArea[" + std::to_string(i) + "]";

			SetUniformi(uName, __TerrainConfig.lodMorphingAreas[i]);
		}

		WaterConfig* config = &__WaterConfig;

		SetUniformf("scaleY", config->displacementScale);
		SetUniform("clipplane", Vector4f(0, -1, 0, 100000));

		SetUniformi("tiling", config->tiling);

		SetUniform("wind", config->windDirection);

		SetUniformf("displacementScale", config->displacementScale);
		SetUniformf("choppiness", config->choppiness);

		SetUniformi("displacementRange", config->displacementRange);
	}

	void GLWaterWireframeShader::UpdateMaterialUniforms(Material* material, uint32_t descriptor_index) const
	{
		TexturesData* texData = (TexturesData*)material;

		texData->Dy->Bind(3);
		texData->Dx->Bind(4);
		texData->Dz->Bind(5);

		SetUniformi("Dy", 3);
		SetUniformi("Dx", 4);
		SetUniformi("Dz", 5);

		SetUniformf("motion", texData->motion);
	}

	void GLWaterWireframeShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		WaterNode* node = (WaterNode*)object;

		SetUniform(location_localMatrix, node->getLocalTransform().getTransformationMatrix());
		SetUniform(location_worldMatrix, node->getWorldTransform().getTransformationMatrix());

		SetUniform(location_location, node->getLocation());
		SetUniform(location_index, node->getIndex());
		SetUniformf(location_gap, node->getGap());
		SetUniformi(location_lod, node->getLod());
	}
}
