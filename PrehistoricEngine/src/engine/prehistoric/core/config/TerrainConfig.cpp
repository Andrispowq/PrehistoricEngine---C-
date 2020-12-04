#include "engine/prehistoric/core/util/Includes.hpp"
#include "TerrainConfig.h"

#include "engine/prehistoric/core/modules/terrain/TerrainQuadtree.h"
#include "engine/prehistoric/core/model/material/Material.h" 

namespace TerrainConfig
{
	float TerrainConfig::scaleY;
	float TerrainConfig::scaleXZ;

	int TerrainConfig::tessellationFactor;
	float TerrainConfig::tessellationSlope;
	float TerrainConfig::tessellationShift;

	std::vector<int> TerrainConfig::lodRanges;
	std::vector<int> TerrainConfig::lodMorphingAreas;

	std::string TerrainConfig::heightmap;
	std::vector<Material*> TerrainConfig::materials;

	void TerrainConfig::LoadConfig(const std::string& path, Window* window, AssetManager* manager)
	{
		std::ifstream file;
		file.open(path.c_str());

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
				else if (nameTokens[0] == "scale")
				{
					if (nameTokens[1] == "y")
					{
						scaleY = (float)std::atof(tokens[1].c_str());
					}
					else if (nameTokens[1] == "xz")
					{
						scaleXZ = (float)std::atof(tokens[1].c_str());
					}
				}
				else if (nameTokens[0] == "tessellation")
				{
					if (nameTokens[1] == "factor")
					{
						tessellationFactor = std::atoi(tokens[1].c_str());
					}
					else if (nameTokens[1] == "slope")
					{
						tessellationSlope = (float) std::atof(tokens[1].c_str());
					}
					else if (nameTokens[1] == "shift")
					{
						tessellationShift = (float) std::atof(tokens[1].c_str());
					}
				}
				else if (nameTokens[0] == "lod")
				{
					if (nameTokens[1] == "ranges")
					{
						lodRanges.resize(std::atoi(tokens[1].c_str()));
						lodMorphingAreas.resize(std::atoi(tokens[1].c_str()));
					}
					else if (nameTokens[1] == "range")
					{
						int index = std::atoi(tokens[1].c_str());
						int range = std::atoi(tokens[2].c_str());
						lodRanges[index] = range;

						if (range == 0)
							lodMorphingAreas[index] = 0;
						else
							lodMorphingAreas[index] = range - UpdateMorphingArea(index + 1);
					}
				}				
				//TODO: make the per-terrain data be in the world files, not in the general terrain-file
				else if (nameTokens[0] == "heightmap")
				{
					heightmap = tokens[1];
				}
				else if (nameTokens[0] == "materials")
				{
					if (nameTokens[1] == "add")
					{
						materials.push_back(new Material(manager, window));
					}
					else
					{
						Material* material = materials[materials.size() - 1];

						if (nameTokens[2] == "texture")
						{
							material->addTexture(tokens[1], manager->getResource<Texture>(tokens[2]));
						}
						else if (nameTokens[2] == "vec4")
						{
							material->addVector4f(tokens[1], Vector4f((float) std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str())
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

			file.close();
		}
		else
		{
			PR_LOG_ERROR("File %s cannot be opened!\n", path.c_str());
		}
	}

	int TerrainConfig::UpdateMorphingArea(int lod)
	{
		return (int)((scaleXZ / TerrainQuadtree::rootNodes) / pow(2, lod));
	}
};