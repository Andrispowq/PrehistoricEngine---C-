#include "engine/prehistoric/core/util/Includes.hpp"
#include "FrameworkConfig.h"

namespace FrameworkConfig
{
	std::string FrameworkConfig::windowName;
	uint32_t FrameworkConfig::windowWidth;
	uint32_t FrameworkConfig::windowHeight;
	bool FrameworkConfig::windowFullScreen;
	bool FrameworkConfig::windowResizable;
	bool FrameworkConfig::windowVSync;
	uint32_t FrameworkConfig::windowNumSamples;
	uint32_t FrameworkConfig::windowMaxFPS;

	API FrameworkConfig::api;
	Vector2i FrameworkConfig::apiVersion;
	bool FrameworkConfig::apiVulkanUseValidationLayers;

	void FrameworkConfig::LoadConfig(const std::string& path)
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

				if (nameTokens[0] == "window")
				{
					if (nameTokens[1] == "name")
					{
						std::string name;

						for (uint32_t i = 1; i < tokens.size(); i++)
						{
							name += tokens[i];
						}

						windowName = name;
					}
					if (nameTokens[1] == "width")
					{
						windowWidth = (uint32_t) std::atoi(tokens[1].c_str());
					}
					if (nameTokens[1] == "height")
					{
						windowHeight = (uint32_t) std::atoi(tokens[1].c_str());
					}
					if (nameTokens[1] == "fullScreen")
					{
						windowFullScreen = std::atoi(tokens[1].c_str()) != 0;
					}
					if (nameTokens[1] == "resizable")
					{
						windowResizable = std::atoi(tokens[1].c_str()) != 0;
					}
					if (nameTokens[1] == "vSync")
					{
						windowVSync = std::atoi(tokens[1].c_str()) != 0;
					}
					if (nameTokens[1] == "numSamples")
					{
						windowNumSamples = std::atoi(tokens[1].c_str());
					}
					if (nameTokens[1] == "maxFPS")
					{
						windowMaxFPS = std::atoi(tokens[1].c_str());
					}
				}
				if (nameTokens[0] == "api")
				{
					if (nameTokens[1] == "name")
					{
						if (tokens[1] == "OpenGL")
						{
							api = OpenGL;
						}
						else if (tokens[1] == "Vulkan")
						{
							api = Vulkan;
						}
						else
						{
							PR_LOG_ERROR("Specify a valid API (OpenGL or Vulkan) under res/config/framework.cfg!\n");
							api = NO_API;
						}
					}
					if (nameTokens[1] == "version")
					{
						apiVersion = { std::atoi(tokens[1].c_str()), std::atoi(tokens[2].c_str()) };
					}
					if (nameTokens[1] == "vulkan")
					{
						if (nameTokens[2] == "useValidationLayers")
						{
							apiVulkanUseValidationLayers = std::atoi(tokens[1].c_str()) != 0;
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
}