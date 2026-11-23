#include "Includes.hpp"
#include "TerrainMaps.h"
#include "platform/vulkan/rendering/VKTerrain.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/core/model/material/Material.h"

#include "prehistoric/application/Application.h"

#include <json.hpp>

namespace Prehistoric
{
	TerrainMaps::TerrainMaps(Window* window, AssembledAssetManager* manager, const std::string& terrainConfigFile)
		: window(window), manager(manager)
	{
		LoadConfigFile(terrainConfigFile);

		heightmap = manager->getAssetManager()->loadTexture(heightmapLocation).value();
		manager->getAssetManager()->addReference<Texture>(heightmap.handle);

		if (__FrameworkConfig.api == Vulkan)
		{
			BakeVulkanTerrainMaps(window, manager, heightmap, normalmap, splatmap, vulkanHeights);
			heights = vulkanHeights.data();
			return;
		}

		this->normalmapRendererComponent = new NormalMapRenderer(window, manager, normalStrength, heightmap->getWidth());
		normalmapRendererComponent->Render(heightmap.pointer);
		this->normalmap = normalmapRendererComponent->getNormalmap();

		this->splatmapRendererComponent = new SplatMapRenderer(window, manager, normalmap->getWidth());
		splatmapRendererComponent->Render(normalmap.pointer);
		this->splatmap = splatmapRendererComponent->getSplatmap();

		this->query = new TerrainHeightsQuery(window, manager, heightmap->getWidth());
		query->Query(heightmap.pointer);
		this->heights = query->getHeights();
	}

	TerrainMaps::~TerrainMaps()
	{
		AssetManager* man = manager->getAssetManager();

		man->removeReference<Texture>(heightmap.handle);

		for (auto mat : materials)
		{
			manager->removeReference<Material>(mat.handle);
		}

		if (!vulkanHeights.empty())
		{
			man->removeReference<Texture>(normalmap.handle);
			man->removeReference<Texture>(splatmap.handle);
		}
		delete normalmapRendererComponent;
		delete splatmapRendererComponent;
		delete query;
	}

	void TerrainMaps::LoadConfigFile(const std::string& terrainConfigFile)
	{
		std::ifstream file;
		file.open(terrainConfigFile.c_str(), std::ios::ate);

		AssetManager* man = manager->getAssetManager();

		if (file.is_open())
		{
			std::string contents;
			size_t size = file.tellg();
			contents.resize(size);
			file.seekg(0);
			file.read(&contents[0], size);

			nlohmann::json file_json = nlohmann::json::parse(contents);

			{
				heightmapLocation = file_json["heightmap"];
				std::vector<float> position_list = file_json["location"];
				position = Vector3f(position_list[0], position_list[1], position_list[2]);
			}

			{
				nlohmann::json materials_json = file_json["materials"];
				for (auto& element : materials_json)
				{
					std::string name = element["name"];
					std::vector<nlohmann::json> contents = element["contents"];

					Material* material = new Material(man);
					for (auto& content : contents)
					{
						std::string name = content["name"];
						std::string type = content["type"];

						if (type == "texture")
						{
							std::string value = content["value"];

							SamplerFilter filter;
							if (name == "albedo")
								filter = Anisotropic;
							else
								filter = Trilinear;

							material->addTexture(name + "Map", man->loadTexture(value, filter, Repeat).value());
						}
						else if (type == "vec4")
						{
							std::vector<float> value = content["value"];
							material->addVector4f(name, Vector4f(value[0], value[1], value[2], value[3]));
						}
						else if (type == "vec3")
						{
							std::vector<float> value = content["value"];
							material->addVector3f(name, Vector3f(value[0], value[1], value[2]));
						}
						else if (type == "vec2")
						{
							std::vector<float> value = content["value"];
							material->addVector2f(name, Vector2f(value[0], value[1]));
						}
						else if (type == "float")
						{
							float value = content["value"];
							material->addFloat(name, value);
						}
						else if (type == "int")
						{
							int value = content["value"];
							material->addInt(name, value);
						}
					}

					materials.push_back(manager->storeMaterial(material));
				}
			}
		}
		else
		{
			PR_LOG_ERROR("Could not open world file %s!\n", terrainConfigFile.c_str());
		}

		for (auto elem : materials)
		{
			manager->addReference<Material>(elem.handle);
		}
	}
};
