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
		rootDomain = mono_jit_init("Prehistoric");
	}

	ScriptEngine::~ScriptEngine()
	{
		mono_jit_cleanup(rootDomain);
	}

	void ScriptEngine::OnEvent(Event& event)
	{
	}

	void ScriptEngine::Update(float delta)
	{
		if (InputInstance.IsKeyPushed(PR_KEY_R)) //Reload scripts
		{
			/*for (auto& elem : scriptObjects)
			{
				std::string dir = elem->getDirectory();
				ScriptComponent::Compile(dir.substr(0, dir.length() - 4), true);
				elem->ReloadAssembly(dir, elem->getComponentName());
				elem->OnInit();
			}*/
		}

		for (auto& elem : scriptObjects)
		{
			void* args[1];
			args[0] = &delta;
			elem->ExecuteFunction("BaseComponent", "Update(single)", (void**)args);
			elem->ExecuteFunction("BaseComponent", "Render()", nullptr);
		}

		scriptObjects.clear();
	}
	
	void ScriptEngine::addScriptObject(ScriptComponent* scriptObject)
	{
		scriptObjects.push_back(scriptObject);
	}
};
