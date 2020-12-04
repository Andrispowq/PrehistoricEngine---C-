#include "engine/prehistoric/core/util/Includes.hpp"
#include "EngineConfig.h"

namespace EngineConfig
{
	uint32_t EngineConfig::lightsMaxNumber;
	float EngineConfig::rendererGamma;
	float EngineConfig::rendererExposure;
	int EngineConfig::rendererHighDetailRange;
	float EngineConfig::rendererNearPlane;
	float EngineConfig::rendererFarPlane;

	void EngineConfig::LoadConfig(const std::string& path)
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
				else if (nameTokens[0] == "lights")
				{
					if (nameTokens[1] == "maxNumber")
					{
						lightsMaxNumber = std::atoi(tokens[1].c_str());
					}
				}
				else if (nameTokens[0] == "renderer")
				{
					if (nameTokens[1] == "gamma")
					{
						rendererGamma = (float) std::atof(tokens[1].c_str());
					}
					else if (nameTokens[1] == "exposure")
					{
						rendererExposure = (float)std::atof(tokens[1].c_str());
					}
					else if (nameTokens[1] == "highDetailRange")
					{
						rendererHighDetailRange = std::atoi(tokens[1].c_str());
					}
					else if (nameTokens[1] == "nearPlane")
					{
						rendererNearPlane = (float)std::atof(tokens[1].c_str());
					}
					else if (nameTokens[1] == "farPlane")
					{
						rendererFarPlane = (float)std::atof(tokens[1].c_str());
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