#ifndef CORE_ENGINE_H
#define CORE_ENGINE_H

#include "prehistoric/core/layers/Layer.h"

#include "prehistoric/core/engines/RenderingEngine.h"
#include "prehistoric/core/engines/AudioEngine.h"

#include "prehistoric/core/scene/Scene.h"

#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/core/config/EngineConfig.h"
#include "prehistoric/core/config/FrameworkConfig.h"
#include "prehistoric/core/config/TerrainConfig.h"
#include "prehistoric/core/config/AtmosphereConfig.h"
#include "prehistoric/core/config/EnvironmentMapConfig.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	constexpr static double NANOSECOND = 1000000000;

	class CoreEngine : public Layer
	{
	public:
		CoreEngine(const std::string& configPath = "res/config", const std::string& worldFile = "res/world/testLevel.wrld");
		~CoreEngine();

		virtual void OnEnable() override {}
		virtual void OnDisable() override {}

		virtual void OnEvent(Event& event) override;

		virtual void Update(float delta);
		virtual void Render();

		void LoadConfigurationFiles(const std::string& path);
		void LoadEngines();

		void AddGameObject(const std::string& name, GameObject* gameobject);

		/*
			Replaces the current scene with the loaded one. Does not delete the old one, and does no management whatsoever -> this is the client's job
		*/
		void SetScene(Scene* scene);

		inline RenderingEngine* getRenderingEngine() const { return renderingEngine.get(); }
		inline AudioEngine* getAudioEngine() const { return audioEngine.get(); }

		inline GameObject* getRootObject() const { return root.get(); }
		inline AssembledAssetManager* getAssetManager() const { return manager.get(); }

		inline float getFrameTime() const { return (float)frameTime; }

		CoreEngine(const CoreEngine& engine) = delete;
		CoreEngine operator=(const CoreEngine& engine) = delete;
		CoreEngine(const CoreEngine&& engine) = delete;
		CoreEngine operator=(const CoreEngine&& engine) = delete;

	private:
		//Root object
		std::unique_ptr<GameObject> root;

		//The asset manager
		std::unique_ptr<AssembledAssetManager> manager;

		//Engines
		std::unique_ptr<RenderingEngine> renderingEngine;
		std::unique_ptr<AudioEngine> audioEngine;

		//Our currently loaded scene, can be changed at any time
		GameObject* sceneRoot;
		Scene* scene;

		//Frametime, set once per update
		bool running;
		double frameTime;
	};
};

#endif