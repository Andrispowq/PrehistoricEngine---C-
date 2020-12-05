#include "Includes.hpp"
#include "AssetManager.h"

namespace Prehistoric
{
	AssetManager::AssetManager(Window* window)
	{
		this->window = window;

		textures.reserve(TexturesSize);
		vertexBuffers.reserve(VertexBuffersSize);
		shaders.reserve(ShadersSize);
	}
};