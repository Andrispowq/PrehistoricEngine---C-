#include "Includes.hpp"
#include "ScriptEngine.h"

#include "mono/metadata/assembly.h"
#include "mono/metadata/debug-helpers.h"

#include <filesystem>

#include "prehistoric/core/node/component/scripting/InternalCalls.h"

#include "prehistoric/application/Application.h"

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

	void ScriptEngine::LoadScripts(const std::string& directory)
	{
		assembly = mono_domain_assembly_open(rootDomain, directory.c_str());
		image = mono_assembly_get_image(assembly);

		InternalCalls::RegisterInternalCalls();
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

	void ScriptEngine::Compile(std::string directory, bool del)
	{
		//We can't check yet if we need to recompile
		std::filesystem::path path = std::filesystem::current_path().append(directory + ".dll");
		std::filesystem::path copy_path = path.parent_path().append("temp/temporary.dll");
		if (std::filesystem::exists(path) && del)
		{
			std::filesystem::copy(path, copy_path);
			std::filesystem::remove(path);
			std::filesystem::copy(copy_path, path);
			std::filesystem::remove(copy_path);
		}

		std::string command = "call \"../PrehistoricEngine/vendor/Mono/bin/mcs\" ../PrehistoricEngine/scripting/Support/*.cs " + directory + "/*.cs -target:library -unsafe -out:" + directory + "/Scripts.dll";
		system(command.c_str());

		Application::Get().getEngineLayer()->getScriptEngine()->LoadScripts(directory + "/Scripts.dll");
	}
};
