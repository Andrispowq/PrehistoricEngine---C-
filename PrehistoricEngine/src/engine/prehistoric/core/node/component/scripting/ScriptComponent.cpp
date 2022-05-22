#include "Includes.hpp"
#include "ScriptComponent.h"

#include "prehistoric/application/Application.h"

#include "mono/metadata/assembly.h"
#include "mono/metadata/debug-helpers.h"

namespace Prehistoric
{
	ScriptComponent::ScriptComponent(std::string directory)
	{
		domain = Application::Get().getEngineLayer()->getScriptEngine()->GetDomain();

		if (directory != "")
		{
			ReloadAssembly(directory);
		}
	}
	
	void ScriptComponent::ReloadAssembly(std::string directory)
	{
		assembly = mono_domain_assembly_open(domain, directory.c_str());
		image = mono_assembly_get_image(assembly);
	}

	void ScriptComponent::PreUpdate(CoreEngine* engine)
	{
		engine->getScriptEngine()->addScriptObject(this);
	}

	void ScriptComponent::ExecuteFunction(std::string object, std::string function, void** parameters)
	{
		std::string method_desc = object + ":" + function;
		MonoMethodDesc* type_method_desc = mono_method_desc_new(method_desc.c_str(), NULL);
		if (!type_method_desc)
		{
			PR_LOG_RUNTIME_ERROR("ERROR: couldn't get function description of %s!\n", method_desc);
		}

		MonoMethod* method = mono_method_desc_search_in_image(type_method_desc, image);
		if (!method)
		{
			PR_LOG_RUNTIME_ERROR("ERROR: couldn't find function %s!\n", method_desc);
		}

		mono_runtime_invoke(method, nullptr, parameters, nullptr);
	}

	void ScriptComponent::Compile(std::string directory)
	{
	}
};
