#include "Includes.hpp"
#include "FrameworkConfig.h"

namespace Prehistoric
{
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

		std::map<API, Vector2i> apiVersions;

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
					else if (nameTokens[0] == "window")
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
						else if (nameTokens[1] == "width")
						{
							windowWidth = (uint32_t)std::atoi(tokens[1].c_str());
						}
						else if (nameTokens[1] == "height")
						{
							windowHeight = (uint32_t)std::atoi(tokens[1].c_str());
						}
						else if (nameTokens[1] == "fullScreen")
						{
							windowFullScreen = std::atoi(tokens[1].c_str()) != 0;
						}
						else if (nameTokens[1] == "resizable")
						{
							windowResizable = std::atoi(tokens[1].c_str()) != 0;
						}
						else if (nameTokens[1] == "vSync")
						{
							windowVSync = std::atoi(tokens[1].c_str()) != 0;
						}
						else if (nameTokens[1] == "numSamples")
						{
							windowNumSamples = std::atoi(tokens[1].c_str());
						}
						else if (nameTokens[1] == "maxFPS")
						{
							windowMaxFPS = std::atoi(tokens[1].c_str());
						}
					}
					else if (nameTokens[0] == "api")
					{
						if (nameTokens[1] == "use")
						{
							std::string name = tokens[1];
							API api = NO_API;

							if (name == "OpenGL")
							{
								api = OpenGL;
							}
							else if(name == "Vulkan")
							{
								api = Vulkan;
							}
							else if (name == "Vulkan_RTX")
							{
								api = Vulkan_RTX;
							}
							else if (name == "DirectX_11")
							{
								api = DirectX_11;
							}
							else if (name == "DirectX_12")
							{
								api = DirectX_12;
							}
							else if (name == "DirectX_DXR")
							{
								api = DirectX_DXR;
							}
							else
							{
								api = NO_API;
								PR_LOG_RUNTIME_ERROR("ERROR: No API is currently not supported in the engine!\n");
							}

							FrameworkConfig::api = api;
							apiVersion = apiVersions[api];
						}
						else
						{
							std::string name = nameTokens[1];
							API api = NO_API;

							if (name == "OpenGL")
							{
								api = OpenGL;
							}
							else if (name == "Vulkan")
							{
								api = Vulkan;
							}
							else if (name == "Vulkan_RTX")
							{
								api = Vulkan_RTX;
							}
							else if (name == "DirectX_11")
							{
								api = DirectX_11;
							}
							else if (name == "DirectX_12")
							{
								api = DirectX_12;
							}
							else if (name == "DirectX_DXR")
							{
								api = DirectX_DXR;
							}

							if (nameTokens[2] == "version")
							{
								Vector2i version{ std::atoi(tokens[1].c_str()), std::atoi(tokens[2].c_str()) };
								apiVersions.insert(std::make_pair(api, version));
							}
							else if (nameTokens[2] == "useValidationLayers")
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
	};
};