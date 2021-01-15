#include "EditorApp.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/core/config/FrameworkConfig.h"
#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"

EditorApp::EditorApp()
{
	using namespace Prehistoric;

	//Load in the environment map
	if (FrameworkConfig::api == OpenGL)
	{
		{
			PR_PROFILE("Environment map generation - BRDF Look-up Table");
			EnvironmentMapRenderer::instance = new EnvironmentMapRenderer(engineLayer->getRenderingEngine()->getWindow(), engineLayer->getAssetManager());
		}

		{
			PR_PROFILE("Environment map generation - Cubemap, Irradiance, Prefilter map");
			EnvironmentMapRenderer::instance->GenerateEnvironmentMap();
		}
	}

	editor = new EditorLayer();
	PushLayer(editor);
}

EditorApp::~EditorApp()
{
}
