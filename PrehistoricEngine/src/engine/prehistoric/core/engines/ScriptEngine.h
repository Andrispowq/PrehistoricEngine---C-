#ifndef SCRIPT_ENGINE_H
#define SCRIPT_ENGINE_H

#include "Engine.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/core/config/EngineSettings.h"

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/common/rendering/Renderer.h"

#include "prehistoric/core/node/component/scripting/ScriptComponent.h"

#include <mono/jit/jit.h>

namespace Prehistoric
{
	class ScriptEngine : public Engine
	{
	public:
		ScriptEngine();
		virtual ~ScriptEngine();

		void LoadScripts(const std::string& directory);

		virtual void OnEvent(Event& event) override;
		virtual void Update(float delta) override;

		void addScriptObject(ScriptComponent* scriptObject);

		MonoDomain* getRootDomain() const { return rootDomain; }
		MonoAssembly* getAssembly() const { return assembly; }
		MonoImage* getImage() const { return image; }

		ScriptEngine(const ScriptEngine& engine) = delete;
		ScriptEngine operator=(const ScriptEngine& engine) = delete;

	public:
		static void Compile(std::string directory, bool del = false);

	private:
		MonoDomain* rootDomain = nullptr;
		MonoAssembly* assembly = nullptr;
		MonoImage* image;

		std::vector<ScriptComponent*> scriptObjects;
	};
};

#endif