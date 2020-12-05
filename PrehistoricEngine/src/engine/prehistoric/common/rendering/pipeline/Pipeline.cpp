#include "engine/prehistoric/core/util/Includes.hpp"
#include "Pipeline.h"

#include "engine/prehistoric/core/config/FrameworkConfig.h"
#include "engine/prehistoric/core/resources/AssetManager.h"

namespace Prehistoric
{
	Pipeline::Pipeline(Window* window, AssetManager* manager, size_t shaderID)
		: viewportStart{ 0, 0 }, viewportSize{ (float)FrameworkConfig::windowWidth, (float)FrameworkConfig::windowHeight },
		scissorStart{ 0, 0 }, scissorSize{ FrameworkConfig::windowWidth, FrameworkConfig::windowHeight }
	{
		this->window = window;
		this->assetManager = manager;

		this->shaderID = shaderID;
		manager->addReference<Shader>(shaderID);
	}

	Pipeline::~Pipeline()
	{
		assetManager->removeReference<Shader>(shaderID);
		shaderID = -1;
	}

	Shader* Pipeline::getShader() const
	{
		return assetManager->getResourceByID<Shader>(shaderID);
	}

	void Pipeline::setShaderID(size_t shaderID)
	{
		this->shaderID = shaderID;
	}
};
