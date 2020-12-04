#include "engine/prehistoric/core/util/Includes.hpp"
#include "AssembledAssetManager.h"

AssembledAssetManager::AssembledAssetManager(Window* window)
	: window(window), manager{ std::make_unique<AssetManager>(window) }
{
	pipelines.reserve(PipelinesSize);
	materials.reserve(MaterialsSize);
}
