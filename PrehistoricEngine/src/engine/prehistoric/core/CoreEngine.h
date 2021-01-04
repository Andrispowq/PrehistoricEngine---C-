#ifndef CORE_ENGINE_H
#define CORE_ENGINE_H

#include "prehistoric/core/engines/RenderingEngine.h"
#include "prehistoric/core/engines/AudioEngine.h"

#include "prehistoric/core/scene/Scene.h"

#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/core/config/EngineConfig.h"
#include "prehistoric/core/config/FrameworkConfig.h"
#include "prehistoric/core/config/TerrainConfig.h"
#include "prehistoric/core/config/AtmosphereConfig.h"
#include "prehistoric/core/config/EnvironmentMapConfig.h"

#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	constexpr static double NANOSECOND = 1000000000;

	class CoreEngine
	{
	public:
		CoreEngine(const std::string& configPath = "res/config", const std::string& worldFile = "res/world/testLevel.wrld");
		~CoreEngine();

		void Start();

		void LoadConfigurationFiles(const std::string& path);
		void LoadEngines();
		void LoadScene(const std::string& worldFile);

		void AddGameObject(const std::string& name, GameObject* gameobject);

		void OnEvent(Event& event);

		inline RenderingEngine* getRenderingEngine() const { return renderingEngine.get(); }
		inline AudioEngine* getAudioEngine() const { return audioEngine.get(); }

		inline ResourceStorage* getResourceStorage() const { return resourceStorage.get(); }

		inline float getFrameTime() const { return (float)frameTime; }

		CoreEngine(const CoreEngine& engine) = delete;
		CoreEngine operator=(const CoreEngine& engine) = delete;
		CoreEngine(const CoreEngine&& engine) = delete;
		CoreEngine operator=(const CoreEngine&& engine) = delete;
	private:
		void Run();
		void Stop();

		void Update(float frameTime);
		void Render();
	private:
		//Root object
		std::unique_ptr<GameObject> root;
		std::unique_ptr<Scene> scene;

		//The asset manager
		std::unique_ptr<ResourceStorage> resourceStorage;

		//Engines
		std::unique_ptr<RenderingEngine> renderingEngine;
		std::unique_ptr<AudioEngine> audioEngine;

		//Frametime, set once per update
		bool running;
		double frameTime;
	};
};

#endif