#ifndef RENDER_STAGE_H
#define RENDER_STAGE_H

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/core/node/movement/Camera.h"

#include "prehistoric/common/rendering/pipeline/Pipeline.h"

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;

	//This class represents a stage of the rendering pipeline, like alpha pass, g-pass, anti-aliasing, post-processing stages, etc
	//It has a single method called Render(). The stage's constructor should already have everything needed for the rendering available
	//so we can just call it when appropriate
	class RenderStage
	{
	public:
		RenderStage(Renderer* renderer);
		virtual ~RenderStage() {}

		virtual void OnResized() = 0;
		virtual void Render() = 0;

	protected:
		AssembledAssetManager* manager;
		Renderer* renderer;
		Window* window;
	};
};

#endif