#include "Includes.hpp"
#include "ScriptComponent.h"

#include "prehistoric/application/Application.h"

#include "mono/metadata/assembly.h"
#include "mono/metadata/debug-helpers.h"

#include <filesystem>

namespace Prehistoric
{
	enum class CallbackType : int
	{
		MOVE = 0x0
	};

	struct CallbackData
	{
		CallbackType type;
		void* data;
	};

	static ScriptComponent* current = nullptr;
	extern "C" void __declspec(dllexport) ScriptCallback(CallbackData* data)
	{
		switch (data->type)
		{
		case CallbackType::MOVE:
		{
			float* data_ptr = (float*)data->data;
			Vector3f amount = Vector3f(data_ptr[0], data_ptr[1], data_ptr[2]);
			current->getParent()->Move(amount);

			break;
		}
		default:
			break;
		}
	}

	ScriptComponent::ScriptComponent(std::string directory)
		: assembly{nullptr}, image{nullptr}, component_class{nullptr}, obj{nullptr}
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

		//Get the class
		component_class = mono_class_from_name(image, "", "ExampleComponent");
		if (!component_class)
		{
			PR_LOG_RUNTIME_ERROR("ERROR: couldn't find class ExampleComponent!\n");
		}

		obj = mono_object_new(domain, component_class);
		if (!obj)
		{
			PR_LOG_RUNTIME_ERROR("ERROR: couldn't initialise class ExampleComponent!\n");
		}

		mono_runtime_object_init(obj);
	}

	void ScriptComponent::PreUpdate(CoreEngine* engine)
	{
		engine->getScriptEngine()->addScriptObject(this);
	}

	void ScriptComponent::ExecuteFunction(std::string object, std::string function, void** parameters)
	{
		std::string method_name = object + ":" + function;
		MonoMethodDesc* method_desc = mono_method_desc_new(method_name.c_str(), NULL);
		if (!method_desc)
		{
			PR_LOG_RUNTIME_ERROR("ERROR: couldn't get function description of %s!\n", method_name.c_str());
		}

		MonoMethod* method = mono_method_desc_search_in_image(method_desc, image);
		if (!method)
		{
			PR_LOG_RUNTIME_ERROR("ERROR: couldn't find function %s!\n", method_name.c_str());
		}

		current = this;
		MonoObject* result = mono_runtime_invoke(method, obj, parameters, nullptr);
	}

	void ScriptComponent::Compile(std::string directory)
	{
		if (std::filesystem::exists(std::filesystem::current_path().append(directory + ".dll")))
		{
		}

		std::string command = "call \"../PrehistoricEngine/vendor/Mono/bin/mcs\" ../PrehistoricEngine/scripting/Support/*.cs " + directory + ".cs -target:library -unsafe -out:" + directory + ".dll";
		system(command.c_str());
	}
};
