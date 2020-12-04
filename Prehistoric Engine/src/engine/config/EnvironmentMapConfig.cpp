#include "engine/prehistoric/core/util/Includes.hpp"
#include "EnvironmentMapConfig.h"

namespace EnvironmentMapConfig
{
	std::string environmentMapLocation;
	uint32_t environmentMapResolution;
	uint32_t irradianceMapResolution;
	uint32_t prefilterMapResolution;
	uint32_t prefilterLevels;

	Texture* environmentMap;
	Texture* irradianceMap;
	Texture* prefilterMap;
	Texture* brdfLUT;

	void EnvironmentMapConfig::LoadConfig(const std::string & path)
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

				if (nameTokens[0] == "environment")
				{
					if (nameTokens[1] == "map")
					{
						if (nameTokens[2] == "location")
						{
							environmentMapLocation = tokens[1];
						}
						if (nameTokens[2] == "resolution")
						{
							environmentMapResolution = std::atoi(tokens[1].c_str());
						}
					}
				}
				if (nameTokens[0] == "irradiance")
				{
					if (nameTokens[1] == "map")
					{
						if (nameTokens[2] == "resolution")
						{
							irradianceMapResolution = std::atoi(tokens[1].c_str());
						}
					}
				}
				if (nameTokens[0] == "prefilter")
				{
					if (nameTokens[1] == "map")
					{
						if (nameTokens[2] == "resolution")
						{
							prefilterMapResolution = std::atoi(tokens[1].c_str());
						}
					}
					if (nameTokens[1] == "levels")
					{
						prefilterLevels = std::atoi(tokens[1].c_str());
					}
				}
			}

			file.close();
		}
		else
		{
			PR_LOG_ERROR("File %s cannot be opened!", path.c_str());
		}
	}
};
