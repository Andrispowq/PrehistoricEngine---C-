#include "engine/prehistoric/core/util/Includes.hpp"
#include "AssetManager.h"

AssetManager::AssetManager(Window* window)
{
	this->window = window;

	textures.reserve(TexturesSize);
	vertexBuffers.reserve(VertexBuffersSize);
	shaders.reserve(ShadersSize);
}