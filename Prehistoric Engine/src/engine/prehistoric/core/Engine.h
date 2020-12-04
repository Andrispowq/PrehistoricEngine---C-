#ifndef ENGINE_H
#define ENGINE_H

#include "engine/prehistoric/scene/Scene.h"

#include "engine/prehistoric/engines/RenderingEngine.h"
#include "engine/prehistoric/engines/AudioEngine.h"

#include "engine/prehistoric/core/gameObject/GameObject.h"

#include "engine/config/EngineConfig.h"
#include "engine/config/FrameworkConfig.h"
#include "engine/config/TerrainConfig.h"
#include "engine/config/AtmosphereConfig.h"
#include "engine/config/EnvironmentMapConfig.h"

#include "engine/prehistoric/resources/AssembledAssetManager.h"

typedef void(*CreateSceneFunction)(GameObject* root, Window* window, AssembledAssetManager* manager, Camera* camera);

class Engine
{
public:
	Engine();
	~Engine();

	void LoadScene(CreateSceneFunction function);
	
	void Input();
	void Update(float frameTime);
	void Render();

	inline RenderingEngine* getRenderingEngine() const { return renderingEngine.get(); }
	inline AudioEngine* getAudioEngine() const { return audioEngine.get(); }

	inline AssembledAssetManager* getAssetManager() const { return manager.get(); }

	inline float getFrameTime() const { return frameTime; }

	Engine(const Engine& engine) = delete;
	Engine operator=(const Engine& engine) = delete;
	Engine(const Engine&& engine) = delete;
	Engine operator=(const Engine&& engine) = delete;
private:
	//Root object
	std::unique_ptr<GameObject> root;
	std::unique_ptr<Scene> scene;

	//The asset manager
	std::unique_ptr<AssembledAssetManager> manager;

	//Engines
	std::unique_ptr<RenderingEngine> renderingEngine;
	std::unique_ptr<AudioEngine> audioEngine;

	//Frametime, set once per update
	float frameTime;
};

#endif