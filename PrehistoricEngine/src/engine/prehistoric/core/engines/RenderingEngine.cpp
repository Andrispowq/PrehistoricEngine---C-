#include "engine/prehistoric/core/util/Includes.hpp"
#include "RenderingEngine.h"

#include "engine/prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "engine/prehistoric/core/node/GameObject.h"
#include "engine/prehistoric/common/util/DeviceProperties.h"

#include "engine/platform/opengl/rendering/GLRenderer.h"
#include "engine/platform/vulkan/rendering/VKRenderer.h"

RenderingEngine::RenderingEngine()
	: window(nullptr), camera(nullptr), renderer(nullptr)
{
#if defined(PR_WINDOWS_64)
	window = std::make_unique<WindowsWindow>();
#elif
	PR_LOG_RUNTIME_ERROR("Unsupported platform is being used!\n");
#endif

	if (!window->Create())
	{
		PR_LOG_RUNTIME_ERROR("The creation of the window has failed!\n");
	}

	Capabilities::getInstance()->QueryCapabilities(window->getContext()->getPhysicalDevice());
	DeviceProperties properties;
	properties.ListProperties(*Capabilities::getInstance());

	window->setClearColour({ 0.23f, 0.78f, 0.88f, 1.0f });

	/*if (FrameworkConfig::api == OpenGL)
	{
		camera = std::make_unique<Camera>(5.0f, 50.0f, 0.8f, 80.0f, Vector3f(-178, 102, -47));
		camera->RotateY(-80);
		camera->RotateX(30);
	}
	else
	{*/
		camera = std::make_unique<Camera>(5.0f, 50.0f, 0.8f, 80.0f, Vector3f(0, 5, -2));
	//}
}

RenderingEngine::~RenderingEngine()
{
	//Order is important!
	delete renderer.release();
	
	delete window.release();
	delete camera.release();
}

void RenderingEngine::Init(AssembledAssetManager* manager)
{
	this->manager = manager;

	CameraInput keyInput({ KEY_HELD, PR_KEY_W, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_S, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_D, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_A, PR_JOYSTICK_1 },
		{ KEY_HELD, PR_KEY_UP, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_DOWN, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_RIGHT, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_LEFT, PR_JOYSTICK_1 });

	camera->AddCameraInput(keyInput);

	camera->LogStage();
	camera->setSpeedControl({ MOUSE_SCROLL, PR_KEY_UNKNOWN, PR_JOYSTICK_1 });

	if (FrameworkConfig::api == OpenGL)
	{
		renderer = std::make_unique<GLRenderer>(window.get(), camera.get());
	}
	else if (FrameworkConfig::api == Vulkan)
	{
		renderer = std::make_unique<VKRenderer>(window.get(), camera.get(), manager);
	}
}

void RenderingEngine::Input()
{
	window->Input();
}

void RenderingEngine::Update(float delta)
{
	if (InputInstance.IsKeyPushed(PR_KEY_ESCAPE))
	{
		window->setClosed(true);
	}

	if (InputInstance.IsKeyPushed(PR_KEY_F11))
	{
		window->SetFullscreen(window->IsFullscreen() xor 0x1);
	}

	if (InputInstance.IsKeyPushed(PR_KEY_E))
	{
		renderer->setWireframeMode(renderer->isWireframeMode() xor 0x1);
	}

	camera->Update(window.get(), delta);
}

#include "engine/prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"

void RenderingEngine::Render()
{
	renderer->PrepareRendering();
	if(FrameworkConfig::api == OpenGL)
		EnvironmentMapRenderer::instance->RenderCube(camera.get());
	renderer->Render();
	renderer->EndRendering();
}