#include "Includes.hpp"
#include "Pipeline.h"

#include "prehistoric/core/config/FrameworkConfig.h"
#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	Pipeline::Pipeline(Window* window, ResourceStorage* resourceStorage, ShaderHandle shader)
		: viewportStart{ 0, 0 }, viewportSize{ (float)FrameworkConfig::windowWidth, (float)FrameworkConfig::windowHeight },
		scissorStart{ 0, 0 }, scissorSize{ FrameworkConfig::windowWidth, FrameworkConfig::windowHeight }
	{
		this->window = window;
		this->resourceStorage = resourceStorage;

		this->shader = shader;
		resourceStorage->addReference<Shader>(shader.handle);
	}

	Pipeline::~Pipeline()
	{
		resourceStorage->removeReference<Shader>(shader.handle);
	}
};
