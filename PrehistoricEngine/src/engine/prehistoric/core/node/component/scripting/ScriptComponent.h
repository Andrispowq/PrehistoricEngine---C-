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
		ScriptComponent(std::string directory, std::string componentName);
		~ScriptComponent();

		virtual void OnInit();

		void CreateDomain(std::string name);
		void DestroyDomain();

		void ReloadAssembly(std::string directory, std::string componentName);
		void PreUpdate(CoreEngine* engine) override;

		void ExecuteFunction(std::string object, std::string function, void** parameters);

		std::string getDirectory() const { return directory; }
		std::string getComponentName() const { return componentName; }
	public:
		static void Compile(std::string directory, bool del = false);
	private:
		std::string directory = "";
		std::string componentName = "";

		MonoDomain* domain;

		MonoAssembly* assembly = nullptr;
		MonoImage* image;

		MonoClass* component_class;
		MonoObject* obj;
	};
};

#endif