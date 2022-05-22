#include "Includes.hpp"
#include "ScriptEngine.h"

#include "mono/metadata/assembly.h"
#include "mono/metadata/debug-helpers.h"

namespace Prehistoric
{
	ScriptEngine::ScriptEngine()
	{
		mono_set_dirs("C:\\Program Files\\Mono\\lib",
			"C:\\Program Files\\Mono\\etc");
		domain = mono_jit_init("Prehistoric");
	}

	ScriptEngine::~ScriptEngine()
	{
		mono_jit_cleanup(domain);
	}

	void ScriptEngine::OnEvent(Event& event)
	{
	}

	void ScriptEngine::Update(float delta)
	{
		for (auto& elem : scriptObjects)
		{
			float params[1] = { delta };
			elem->ExecuteFunction("ExampleComponent", "Update(float)", (void**)&params);
		}

		scriptObjects.clear();
	}
	
	void ScriptEngine::addScriptObject(ScriptComponent* scriptObject)
	{
		scriptObjects.push_back(scriptObject);
	}
};
