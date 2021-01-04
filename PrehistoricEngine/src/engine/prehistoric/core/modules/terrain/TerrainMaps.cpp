#include "Includes.hpp"
#include "TerrainMaps.h"

#include "prehistoric/core/resources/ResourceStorage.h"
#include "prehistoric/core/model/material/Material.h"

namespace Prehistoric
{
	TerrainMaps::TerrainMaps(Window* window, ResourceStorage* resourceStorage, const std::string& terrainConfigFile)
	{
		this->resourceStorage = resourceStorage;

		LoadConfigFile(terrainConfigFile);

		heightmap = resourceStorage->loadTexture(heightmapLocation).value();
		resourceStorage->addReference<Texture>(heightmap.handle);

		this->normalmapRendererComponent = new NormalMapRenderer(window, resourceStorage, 60, heightmap->getWidth());
		normalmapRendererComponent->Render(heightmap.pointer);
		this->normalmap = normalmapRendererComponent->getNormalmap();

		this->splatmapRendererComponent = new SplatMapRenderer(window, resourceStorage, normalmap->getWidth());
		splatmapRendererComponent->Render(normalmap.pointer);
		this->splatmap = splatmapRendererComponent->getSplatmap();

		this->query = new TerrainHeightsQuery(window, resourceStorage, heightmap->getWidth());
		query->Query(heightmap.pointer);
		this->heights = query->getHeights();
	}

	TerrainMaps::~TerrainMaps()
	{
		resourceStorage->removeReference<Texture>(heightmap.handle);

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
						materials.push_back(new Material(resourceStorage));
					}
					else
					{
						Material* material = materials[materials.size() - 1];

						if (nameTokens[2] == "texture")
						{
							material->addTexture(tokens[1], resourceStorage->loadTexture(tokens[2]).value());
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
