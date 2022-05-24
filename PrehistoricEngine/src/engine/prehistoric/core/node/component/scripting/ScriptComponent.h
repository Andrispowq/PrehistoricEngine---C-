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
		ScriptComponent(std::string directory = "");
		~ScriptComponent() {}

		void ReloadAssembly(std::string directory);
		void PreUpdate(CoreEngine* engine) override;

		void ExecuteFunction(std::string object, std::string function, void** parameters);
	public:
		static void Compile(std::string directory);
	private:
		MonoDomain* domain;

		MonoAssembly* assembly;
		MonoImage* image;

		MonoClass* component_class;
		MonoObject* obj;
	};
};

#endif