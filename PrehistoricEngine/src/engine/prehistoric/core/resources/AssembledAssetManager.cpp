#include "Includes.hpp"
#include "AssembledAssetManager.h"

namespace Prehistoric
{
	AssembledAssetManager::AssembledAssetManager(Window* window)
		: window(window), manager{ std::make_unique<AssetManager>(window) }
	{
		pipelines.reserve(PipelinesSize);
		materials.reserve(MaterialsSize);
	}
};
