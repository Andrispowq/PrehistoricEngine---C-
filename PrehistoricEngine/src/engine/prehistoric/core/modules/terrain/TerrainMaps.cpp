#include "Includes.hpp"
#include "TerrainMaps.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/core/model/material/Material.h"

namespace Prehistoric
{
	TerrainMaps::TerrainMaps(Window* window, AssembledAssetManager* manager, const std::string& terrainConfigFile)
	{
		this->manager = manager;

		LoadConfigFile(terrainConfigFile);

		heightmap = manager->getAssetManager()->loadTexture(heightmapLocation).value();
		manager->getAssetManager()->addReference<Texture>(heightmap.handle);

		this->normalmapRendererComponent = new NormalMapRenderer(window, manager, 60, heightmap->getWidth());
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
		manager->getAssetManager()->removeReference<Texture>(heightmap.handle);

		delete normalmapRendererComponent;
		delete splatmapRendererComponent;
		delete query;
	}

	void TerrainMaps::LoadConfigFile(const std::string& terrainConfigFile)
	{
		std::ifstream file(terrainConfigFile);
		std::string line;

		size_t textureCount = 0;
		AssetManager* man = manager->getAssetManager();

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
						materials.push_back(manager->storeMaterial(new Material(man)));
					}
					else
					{
						MaterialHandle material = materials[materials.size() - 1];

						if (nameTokens[2] == "texture")
						{
							SamplerFilter filter;
							if (tokens[1] == ALBEDO_MAP)
								filter = Anisotropic;
							else
								filter = Trilinear;

							textureLocations.emplace_back(materials.size() - 1, tokens[1]);
							textureCount++;
							man->loadTexture(tokens[2], filter, Repeat, BatchSettings::QueuedLoad);
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

		man->getTextureLoader()->ForceLoadQueue();

		size_t count;
		Texture** pointers = (Texture**)man->getTextureLoader()->GetLoadedPointers(count);

		if (count != textureCount)
		{
			PR_LOG_ERROR("The loaded texture count isn't equal to the requested texture count!\n");
		}

		for (int i = 0; i < textureCount; i++)
		{
			auto entry = textureLocations[i];
			materials[entry.first]->addTexture(entry.second, man->storeTexture(pointers[i]));
		}

		man->getTextureLoader()->FlushPointers();
	}
};
