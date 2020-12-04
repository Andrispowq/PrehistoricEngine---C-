#include "engine/prehistoric/core/util/Includes.hpp"
#include "AtmosphereConfig.h"

namespace AtmosphereConfig
{
	float AtmosphereConfig::sunRadius;
	Vector3f AtmosphereConfig::sunPosition;
	Vector3f AtmosphereConfig::sunColour;
	float AtmosphereConfig::sunIntensity;

	Vector3f AtmosphereConfig::fogColour;
	float AtmosphereConfig::fogBrightness;

	float AtmosphereConfig::planetRadius;
	float AtmosphereConfig::atmosphereRadius;

	Vector3f AtmosphereConfig::rayleigh;
	float AtmosphereConfig::rayleighHeightScale;
	float AtmosphereConfig::mie;
	float AtmosphereConfig::mieHeightScale;
	float AtmosphereConfig::mieDirection;

	float AtmosphereConfig::sightRange;
	float AtmosphereConfig::ambient;
	float AtmosphereConfig::horizontalVerticalShift;
	float AtmosphereConfig::bloomFactor;
	bool AtmosphereConfig::scatteringEnabled;

	void LoadConfig(const std::string& path)
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

				if (nameTokens[0] == "sun")
				{
					if (nameTokens[1] == "position")
					{
						sunPosition = Vector3f((float)std::atof(tokens[1].c_str()), (float)std::atof(tokens[2].c_str()) , (float)std::atof(tokens[3].c_str()));
					}
					if (nameTokens[1] == "radius")
					{
						sunRadius = (float)std::atof(tokens[1].c_str());
					}
					if (nameTokens[1] == "colour")
					{
						sunColour = Vector3f((float)std::atof(tokens[1].c_str()), (float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str()));
					}
					if (nameTokens[1] == "intensity")
					{
						sunIntensity = (float)std::atof(tokens[1].c_str());
					}
				}

				if (nameTokens[0] == "fog")
				{
					if (nameTokens[1] == "colour")
					{
						fogColour = Vector3f((float)std::atof(tokens[1].c_str()), (float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str()));
					}
					if (nameTokens[1] == "brightness")
					{
						fogBrightness = (float)std::atof(tokens[1].c_str());
					}
				}

				if (nameTokens[0] == "planetRadius")
				{
					planetRadius = (float)std::atof(tokens[1].c_str());
				}
				if (nameTokens[0] == "atmosphereRadius")
				{
					atmosphereRadius = (float)std::atof(tokens[1].c_str());
				}

				if (nameTokens[0] == "coefficients")
				{
					if (nameTokens[1] == "rayleigh")
					{
						rayleigh = Vector3f((float)std::atof(tokens[1].c_str()), (float)std::atof(tokens[2].c_str()), (float)std::atof(tokens[3].c_str()));
					}
					if (nameTokens[1] == "rayleighHeightScale")
					{
						rayleighHeightScale = (float)std::atof(tokens[1].c_str());
					}
					if (nameTokens[1] == "mie")
					{
						mie = (float)std::atof(tokens[1].c_str());
					}
					if (nameTokens[1] == "mieHeightScale")
					{
						mieHeightScale = (float)std::atof(tokens[1].c_str());
					}
					if (nameTokens[1] == "mieDirection")
					{
						mieDirection = (float)std::atof(tokens[1].c_str());
					}
				}

				if (nameTokens[0] == "sightRange")
				{
					sightRange = (float)std::atof(tokens[1].c_str());
				}
				if (nameTokens[0] == "ambient")
				{
					ambient = (float)std::atof(tokens[1].c_str());
				}
				if (nameTokens[0] == "horizontalVerticalShift")
				{
					horizontalVerticalShift = (float)std::atof(tokens[1].c_str());
				}
				if (nameTokens[0] == "bloomFactor")
				{
					bloomFactor = (float)std::atof(tokens[1].c_str());
				}
				if (nameTokens[0] == "scatteringEnabled")
				{
					scatteringEnabled = (float)std::atof(tokens[1].c_str());
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