#include "Includes.hpp"
#include "TerrainConfig.h"

#include "prehistoric/core/modules/terrain/TerrainQuadtree.h"
#include "prehistoric/core/model/material/Material.h" 

namespace Prehistoric
{
	namespace TerrainConfig
	{
		float TerrainConfig::scaleY;
		float TerrainConfig::scaleXZ;

		int TerrainConfig::tessellationFactor;
		float TerrainConfig::tessellationSlope;
		float TerrainConfig::tessellationShift;

		std::vector<int> TerrainConfig::lodRanges;
		std::vector<int> TerrainConfig::lodMorphingAreas;

		std::vector<Material*> TerrainConfig::terrainMaterials;

		Texture* TerrainConfig::heightmap;
		Texture* TerrainConfig::normalmap;
		Texture* TerrainConfig::splatmap;

		void TerrainConfig::LoadConfig(const std::string& path)
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
							tessellationSlope = (float)std::atof(tokens[1].c_str());
						}
						else if (nameTokens[1] == "shift")
						{
							tessellationShift = (float)std::atof(tokens[1].c_str());
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
};