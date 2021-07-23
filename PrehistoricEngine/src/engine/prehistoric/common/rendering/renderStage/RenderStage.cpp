#include "Includes.hpp"
#include "RenderStage.h"

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	RenderStage::RenderStage(Renderer* renderer)
		: window(renderer->getWindow()), camera(renderer->getCamera()), manager(renderer->getAssetManager())
	{
	}
};
