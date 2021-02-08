#include "Includes.hpp"
#include "RenderingEngine.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/vulkan/rendering/VKRenderer.h"

#include "prehistoric/common/util/DeviceProperties.h"

#include "platform/windows/WindowsWindow.h"

namespace Prehistoric
{
	Statistics RenderingEngine::statistics;

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

		window->setClearColour({ 0.23f, 0.78f, 0.88f, 1.0f });


		Capabilities::getInstance()->QueryCapabilities(window->getContext()->getDevices());
		DeviceProperties properties;
		properties.ListProperties(*Capabilities::getInstance());

		CameraInput keyInput({ KEY_HELD, PR_KEY_W, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_S, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_D, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_A, PR_JOYSTICK_1 },
			{ KEY_HELD, PR_KEY_UP, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_DOWN, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_RIGHT, PR_JOYSTICK_1 }, { KEY_HELD, PR_KEY_LEFT, PR_JOYSTICK_1 });

		camera = std::make_unique<Camera>(5.0f, 50.0f, 0.8f, 80.0f, Vector3f(0));
		camera->AddCameraInput(keyInput);

		camera->LogStage();
		camera->setSpeedControl({ MOUSE_SCROLL, PR_KEY_UNKNOWN, PR_JOYSTICK_1 });
	}

	RenderingEngine::~RenderingEngine()
	{
		//Order is important!
		delete renderer.release();
		delete camera.release();
	}

	void RenderingEngine::Init(AssembledAssetManager* manager)
	{
		if (FrameworkConfig::api == OpenGL)
		{
			renderer = std::make_unique<GLRenderer>(window.get(), camera.get(), manager);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			renderer = std::make_unique<VKRenderer>(window.get(), camera.get(), manager);
		}
	}

	void RenderingEngine::OnEvent(Event& event)
	{
		window->OnEvent(event);
	}

	void RenderingEngine::Update(float delta)
	{
		if (InputInstance.IsKeyPushed(PR_KEY_ESCAPE))
		{
			window->setClosed(true);
		}

		if (InputInstance.IsKeyPushed(PR_KEY_F11))
		{
			window->SetFullscreen(!window->IsFullscreen());
		}

		if (InputInstance.IsKeyPushed(PR_KEY_E))
		{
			renderer->setWireframeMode(!renderer->isWireframeMode());
		}

		camera->Update(window.get(), delta);
	}

	void RenderingEngine::Render()
	{
		renderer->Render();
	}
};