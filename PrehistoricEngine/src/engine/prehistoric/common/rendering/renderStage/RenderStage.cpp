#include "Includes.hpp"
#include "RenderStage.h"

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	RenderStage::RenderStage(Renderer* renderer)
		: renderer(renderer), window(renderer->getWindow()), manager(renderer->getAssetManager())
	{
	}
};
