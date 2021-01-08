#include "Includes.hpp"
#include "Pipeline.h"

#include "prehistoric/core/config/FrameworkConfig.h"
#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	Pipeline::Pipeline(Window* window, AssetManager* manager, ShaderHandle shader)
		: viewportStart{ 0, 0 }, viewportSize{ (float)FrameworkConfig::windowWidth, (float)FrameworkConfig::windowHeight },
		scissorStart{ 0, 0 }, scissorSize{ FrameworkConfig::windowWidth, FrameworkConfig::windowHeight }
	{
		this->window = window;
		this->manager = manager;

		this->shader = shader;
		manager->addReference<Shader>(shader.handle);
	}

	Pipeline::~Pipeline()
	{
		manager->removeReference<Shader>(shader.handle);
	}
};
