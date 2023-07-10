#include "Includes.hpp"
#include "ScriptComponent.h"

#include "prehistoric/application/Application.h"

#include "mono/metadata/assembly.h"
#include "mono/metadata/debug-helpers.h"

#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/core/node/component/light/Light.h"
#include "prehistoric/core/node/component/camera/CameraComponent.h"

#include <locale>
#include <codecvt>

namespace Prehistoric
{	
	GameObject* current_parent = nullptr;

	ScriptComponent::ScriptComponent(std::string componentName)
		: component_class{nullptr}, obj{nullptr}
	{
		ScriptEngine* scriptEngine = Application::Get().getEngineLayer()->getScriptEngine();
		domain = scriptEngine->getRootDomain();
		assembly = scriptEngine->getAssembly();
		image = scriptEngine->getImage();

		if (componentName != "")
		{
			ReloadAssembly(componentName);
		}
	}

	ScriptComponent::~ScriptComponent()
	{
	}
	
	void ScriptComponent::OnInit()
	{
		current_parent = parent;
		ExecuteFunction("BaseComponent", "Init()", nullptr);
	}

	void ScriptComponent::ReloadAssembly(std::string componentName)
	{
		this->componentName = componentName;

		//Get the class
		component_class = mono_class_from_name(image, "", componentName.c_str());
		if (!component_class)
		{
			PR_LOG_RUNTIME_ERROR("ERROR: couldn't find class %s!\n", &componentName[0]);
		}

		obj = mono_object_new(domain, component_class);
		if (!obj)
		{
			PR_LOG_RUNTIME_ERROR("ERROR: couldn't initialise class %s!\n", &componentName[0]);
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
		MonoMethod* method;

		auto idx = method_table.find(method_name);
		if (idx != method_table.end())
		{
			method = idx->second;
		}
		else
		{
			MonoMethodDesc* method_desc = mono_method_desc_new(method_name.c_str(), NULL);
			if (!method_desc)
			{
				PR_LOG_RUNTIME_ERROR("ERROR: couldn't get function description of %s!\n", method_name.c_str());
			}

			method = mono_method_desc_search_in_image(method_desc, image);
			if (!method)
			{
				PR_LOG_RUNTIME_ERROR("ERROR: couldn't find function %s!\n", method_name.c_str());
			}

			method_table.insert(std::make_pair(method_name, method));
		}

		if (obj->vtable == nullptr)
		{
			DEBUG_BREAK();
		}

		current_parent = this->parent;
		MonoObject* exception = nullptr;
		MonoObject* result = mono_runtime_invoke(method, obj, parameters, &exception);

		if (exception != nullptr)
		{
			//very sad, can't do much about it right now
			PR_LOG_WARNING("Exception found in ScriptComponent (%s)!\n", &componentName[0]);
			mono_unhandled_exception(exception);
		}
	}
};
