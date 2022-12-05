#ifndef SCRIPT_COMPONENT_H
#define SCRIPT_COMPONENT_H

#include "prehistoric/core/node/component/Component.h"

#include <mono/jit/jit.h>

namespace Prehistoric
{
	class CoreEngine;

	class ScriptComponent : public Component
	{
	public:
		ScriptComponent(std::string componentName);
		~ScriptComponent();

		virtual void OnInit();

		void ReloadAssembly(std::string componentName);
		void PreUpdate(CoreEngine* engine) override;

		void ExecuteFunction(std::string object, std::string function, void** parameters);

		std::string getComponentName() const { return componentName; }
	private:
		std::string componentName = "";

		MonoDomain* domain;
		MonoAssembly* assembly;
		MonoImage* image;

		MonoClass* component_class;
		MonoObject* obj;
	};
};

#endif