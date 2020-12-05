#ifndef ENGINE_H
#define ENGINE_H

#include "engine/prehistoric/core/scene/Scene.h"

#include "engine/prehistoric/core/engines/RenderingEngine.h"
#include "engine/prehistoric/core/engines/AudioEngine.h"

#include "engine/prehistoric/core/node/GameObject.h"

#include "engine/prehistoric/core/config/EngineConfig.h"
#include "engine/prehistoric/core/config/FrameworkConfig.h"
#include "engine/prehistoric/core/config/TerrainConfig.h"
#include "engine/prehistoric/core/config/AtmosphereConfig.h"
#include "engine/prehistoric/core/config/EnvironmentMapConfig.h"

#include "engine/prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
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
};

#endif