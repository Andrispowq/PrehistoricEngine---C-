#include "EditorApp.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/application/Application.h"

#include "prehistoric/core/node/movement/EditorCamera.h"

EditorApp::EditorApp()
{
	using namespace Prehistoric;

	//AssembledAssetManager -> stores the primitives of rendering (Pipelines, Materials) in one place
	//AssetManager -> store the assembling blocks of the primitives (Textures, VertexBuffers, Shaders) in one place
	//Window -> used in a lot of primitives' creation, so it's worth having it around
	AssembledAssetManager* manager = engineLayer->getAssetManager();
	AssetManager* man = manager->getAssetManager();
	Window* window = engineLayer->getRenderingEngine()->getWindow();
	GameObject* root = engineLayer->getRootObject();

	Camera* cam = engineLayer->getRenderingEngine()->getCamera();//new EditorCamera(); 
	//engineLayer->getRenderingEngine()->ChangeCamera(cam);

	cam->setPosition(Vector3f(0, 5, -2));
	cam->Update(engineLayer->getRenderingEngine()->getWindow(), 0.0f);

	auto scene = std::make_unique<Scene>("res/world/testLevel.wrld", window, cam, manager);
	scene->getSceneRoot()->AddChild("sun", Atmosphere::CreateSun().release());
	engineLayer->SetScene(scene.get());

	editor = new EditorLayer(std::move(scene));
	PushLayer(editor);
}

EditorApp::~EditorApp()
{
	layerStack.PopLayer(editor);
	delete editor;
}
