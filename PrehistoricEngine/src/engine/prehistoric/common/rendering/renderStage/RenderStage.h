#ifndef RENDER_STAGE_H
#define RENDER_STAGE_H

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/core/node/movement/Camera.h"

#include "prehistoric/common/rendering/pipeline/Pipeline.h"

namespace Prehistoric
{
	class RenderableComponent;
	class AssembledAssetManager;

	//This class represents a stage of the rendering pipeline, like alpha pass, g-pass, anti-aliasing, post-processing stages, etc
	//It has a single method called Render(). The stage's constructor should already have everything needed for the rendering available
	//so we can just call it when appropriate
	class RenderStage
	{
	public:
		RenderStage(Window* window, Camera* camera, AssembledAssetManager* manager);
		virtual ~RenderStage() {}

		virtual void Render();

	protected:
		AssembledAssetManager* manager;

		Window* window;
		Camera* camera;
	};
};

#endif