#include "Includes.hpp"
#include "TerrainMaps.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	TerrainMaps::TerrainMaps(Window* window, AssembledAssetManager* manager, const std::string& terrainConfigFile)
	{
		this->manager = manager;

		LoadConfigFile(terrainConfigFile);

		heightmapID = manager->getAssetManager()->getResource<Texture>(heightmapLocation);
		manager->getAssetManager()->addReference<Texture>(heightmapID);
		this->heightmap = manager->getAssetManager()->getResourceByID<Texture>(heightmapID);

		this->normalmapRendererComponent = new NormalMapRenderer(window, manager, 60, heightmap->getWidth());
		normalmapRendererComponent->Render(heightmap);
		this->normalmap = normalmapRendererComponent->getNormalmap();

		this->splatmapRendererComponent = new SplatMapRenderer(window, manager, normalmap->getWidth());
		splatmapRendererComponent->Render(normalmap);
		this->splatmap = splatmapRendererComponent->getSplatmap();

		this->query = new TerrainHeightsQuery(window, manager, heightmap->getWidth());
		query->Query(heightmap);
		this->heights = query->getHeights();
	}

	TerrainMaps::~TerrainMaps()
	{
		manager->getAssetManager()->removeReference<Texture>(heightmapID);

		delete normalmapRendererComponent;
		delete splatmapRendererComponent;
		delete query;
	}

	void TerrainMaps::LoadConfigFile(const std::string& terrainConfigFile)
	{
		std::ifstream file(terrainConfigFile);
		std::string line;

		if (file.is_open())
		{
			while (file.good())
			{
				std::getline(file, line);
				std::vector<std::string> tokens = Util::Split(line, ' ');
				std::vector<std::string> nameTokens = Util::Split(tokens[0], '.');

				if (line.substr(0, 1).c_str()[0] == '#')
				{
					continue; //This line is a comment then
				}
				//TODO: make the per-terrain data be in the world files, not in the general terrain-file
				else if (nameTokens[0] == "heightmap")
				{
					heightmapLocation = tokens[1];
				}
				else if (nameTokens[0] == "materials")
				{
					if (nameTokens[1] == "add")
					{
						materials.push_back(new Material(manager->getAssetManager()));
					}
					else
					{
						Material* material = materials[materials.size() - 1];

						if (nameTokens[2] == "texture")
						{
							material->addTexture(tokens[1], manager->getAssetManager()->getResource<Texture>(tokens[2]));
						}
						else if (nameTokens[2] == "vec4")
						{
							material->addVector4f(tokens[1], Vector4f((float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str())
								, (float)std::atof(tokens[4].c_str()), (float)std::atof(tokens[5].c_str())));
						}
						else if (nameTokens[2] == "vec3")
						{
							material->addVector3f(tokens[1], Vector3f((float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str())
								, (float)std::atof(tokens[4].c_str())));
						}
						else if (nameTokens[2] == "vec2")
						{
							material->addVector2f(tokens[1], Vector2f((float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str())));
						}
						else if (nameTokens[2] == "float")
						{
							material->addFloat(tokens[1], (float)std::atof(tokens[2].c_str()));
						}
						else if (nameTokens[2] == "int")
						{
							material->addInt(tokens[1], std::atoi(tokens[2].c_str()));
						}
					}
				}
			}
		}
	}
};
