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

#define ToComponentType(x) x##asd

namespace Prehistoric
{
	enum class CallbackType : int
	{
		HasComponent = 0x0,
		GetComponent = 0x1,
		SetComponent = 0x2,
		GetTransform = 0x3,
		SetTransform = 0x4,
		Log = 0x5,
		InputCheck = 0x6,
		GetGameObject = 0x7,
	};

	struct CallbackData
	{
		CallbackType type;
		void* data;
	};

	static GameObject* current_parent = nullptr;
	void HasComponent(CallbackData* data)
	{
		bool result = false;

		std::wstring comp_name = (wchar_t*)data->data;
		if (comp_name == L"RenderableComponent")
		{
			result = current_parent->HasComponent<RenderableComponent>();
		}
		else if (comp_name == L"RendererComponent")
		{
			result = current_parent->HasComponent<RendererComponent>();
		}
		else if (comp_name == L"Light")
		{
			result = current_parent->HasComponent<Light>();
		}
		else if (comp_name == L"PhysicsComponent")
		{
			result = current_parent->HasComponent<PhysicsComponent>();
		}
		else if (comp_name == L"AudioComponent")
		{
			result = current_parent->HasComponent<AudioComponent>();
		}
		else if (comp_name == L"ScriptComponent")
		{
			result = current_parent->HasComponent<ScriptComponent>();
		}
		else if (comp_name == L"CameraComponent")
		{
			result = current_parent->HasComponent<CameraComponent>();
		}

		int int_res = result;
		data->data = (void*)&int_res;
	}

	void GetComponent(CallbackData* data)
	{
		void* result = nullptr;

		std::wstring comp_name = (wchar_t*)data->data;
		if (comp_name == L"RenderableComponent")
		{
			result = (void*)current_parent->GetComponent<RenderableComponent>();
		}
		else if (comp_name == L"RendererComponent")
		{
			RendererComponent* comp = current_parent->GetComponent<RendererComponent>();
			Material* mat = comp->getMaterial();
			Vector3f colour = mat->getVector3f(COLOUR);
			Vector4f mrot = mat->getVector4f(MROT);

			static float transfer_material[6];
			transfer_material[0] = colour.r;
			transfer_material[1] = colour.g;
			transfer_material[2] = colour.b;
			transfer_material[3] = mrot.r;
			transfer_material[4] = mrot.g;
			transfer_material[5] = mrot.b;

			result = (void*)transfer_material;
		}
		else if (comp_name == L"Light")
		{
			result = (void*)current_parent->GetComponent<Light>();
		}
		else if (comp_name == L"PhysicsComponent")
		{
			result = (void*)current_parent->GetComponent<PhysicsComponent>();
		}
		else if (comp_name == L"AudioComponent")
		{
			result = (void*)current_parent->GetComponent<AudioComponent>();
		}
		else if (comp_name == L"ScriptComponent")
		{
			result = (void*)current_parent->GetComponent<ScriptComponent>();
		}
		else if (comp_name == L"CameraComponent")
		{
			CameraComponent* comp = current_parent->GetComponent<CameraComponent>();
			Camera* cam = comp->getCamera();

			Vector3f position = cam->getPosition();
			Vector3f forward = cam->getForward();
			Vector3f up = cam->getUp();

			static float transfer[9];
			transfer[0] = position.r;
			transfer[1] = position.g;
			transfer[2] = position.b;
			transfer[3] = forward.r;
			transfer[4] = forward.g;
			transfer[5] = forward.b;
			transfer[6] = up.r;
			transfer[7] = up.g;
			transfer[8] = up.b;

			result = (void*)transfer;
		}

		data->data = result;
	}

	void SetComponent(CallbackData* data)
	{
		void* result = nullptr;
		void** ptrs = (void**)data->data;

		std::wstring comp_name = (wchar_t*)ptrs[0];
		if (comp_name == L"RenderableComponent")
		{
			RenderableComponent* comp = current_parent->GetComponent<RenderableComponent>();
		}
		else if (comp_name == L"RendererComponent")
		{
			RendererComponent* comp = current_parent->GetComponent<RendererComponent>();
			Material* mat = comp->getMaterial();

			float* fdata = (float*)ptrs[1];
			Vector3f colour = { fdata[0], fdata[1], fdata[2] };
			Vector4f mrot = { fdata[3], fdata[4], fdata[5], 0.0f };

			if (mat->getVector3f(COLOUR) == Vector3f(-1)) { mat->addVector3f(COLOUR, colour); }
			if (mat->getVector4f(MROT) == Vector4f(-1)) { mat->addVector4f(MROT, mrot); }

			mat->getVector3fs()[COLOUR] = colour;
			mat->getVector4fs()[MROT] = mrot;

			result = nullptr;
		}
		else if (comp_name == L"Light")
		{
			Light* comp = current_parent->GetComponent<Light>();
		}
		else if (comp_name == L"PhysicsComponent")
		{
			PhysicsComponent* comp = current_parent->GetComponent<PhysicsComponent>();
		}
		else if (comp_name == L"AudioComponent")
		{
			AudioComponent* comp = current_parent->GetComponent<AudioComponent>();
		}
		else if (comp_name == L"ScriptComponent")
		{
			ScriptComponent* comp = current_parent->GetComponent<ScriptComponent>();
		}
		else if (comp_name == L"CameraComponent")
		{
			CameraComponent* comp = current_parent->GetComponent<CameraComponent>();
			Camera* cam = comp->getCamera();

			float* fdata = (float*)ptrs[1];
			Vector3f position = { fdata[0], fdata[1], fdata[2] };
			Vector3f forward = { fdata[3], fdata[4], fdata[5] };
			Vector3f up = { fdata[6], fdata[7], fdata[8] };

			cam->setPosition(position);
			cam->setForward(forward);
			cam->setUp(up);
			cam->Update(nullptr, 0.0f);

			result = nullptr;
		}

		data->data = result;
	}

	void GetTransform(CallbackData* data)
	{
		void** ptrs = (void**)data->data;
		GameObject* obj = current_parent;

		if (ptrs != nullptr)
		{
			obj = (GameObject*)ptrs[0];
		}

		Transform& transform = current_parent->getWorldTransform();

		static float fdata[9];
		fdata[0] = transform.getPosition().x;
		fdata[1] = transform.getPosition().y;
		fdata[2] = transform.getPosition().z;
		fdata[3] = transform.getRotation().x;
		fdata[4] = transform.getRotation().y;
		fdata[5] = transform.getRotation().z;
		fdata[6] = transform.getScaling().x;
		fdata[7] = transform.getScaling().y;
		fdata[8] = transform.getScaling().z;

		data->data = (void*)fdata;
	}

	void SetTransform(CallbackData* data)
	{
		Transform& transform = current_parent->getWorldTransform();

		float* fdata = (float*)data->data;
		transform.setPosition({ fdata[0], fdata[1], fdata[2] });
		transform.setRotation({ fdata[3], fdata[4], fdata[5] });
		transform.setScaling({ fdata[6], fdata[7], fdata[8] });

		data->data = nullptr;
	}

	void ScriptLog(CallbackData* data)
	{
		PR_LOG_MESSAGE("[%s - ScriptComponent]: %s\n", current_parent->getName().c_str(), (char*)data->data);
	}

	void InputCheck(CallbackData* data)
	{
		int* values = (int*)data->data;
		int type = values[0];
		int val = values[1];

		float ret = false;
		if ((type & 0xF) == 0x0) //keyboard
		{
			if (((type >> 4) & 0x7) == 0x0) //pressed
			{
				bool res = InputInstance.IsKeyPushed((InputCode)val);
				ret = (res ? 1.0f : 0.0f);
			}
			else if (((type >> 4) & 0x7) == 0x1) //held
			{
				bool res = InputInstance.IsKeyHeld((InputCode)val);
				ret = (res ? 1.0f : 0.0f);
			}
			else if (((type >> 4) & 0x7) == 0x2) //released - not implemented
			{
				bool res = false;
				ret = (res ? 1.0f : 0.0f);
			}
		}
		else if ((type & 0xF) == 0x1) //mouse
		{
			if (((type >> 4) & 0xF) == 0x0) //pressed
			{
				bool res = InputInstance.IsButtonPushed((InputCode)val);
				ret = (res ? 1.0f : 0.0f);
			}
			else if (((type >> 4) & 0xF) == 0x1) //held
			{
				bool res = InputInstance.IsButtonHeld((InputCode)val);
				ret = (res ? 1.0f : 0.0f);
			}
			else if (((type >> 4) & 0xF) == 0x2) //released - not implemented
			{
				bool res = false;
				ret = (res ? 1.0f : 0.0f);
			}
		}
		else if ((type & 0xF) == 0x2) //joystick
		{
			if (((type >> 4) & 0x7) == 0x0) //pressed
			{
				if (((type >> 7) & 0x1) == 0x1) //axis
				{
					ret = InputInstance.getJoystickAxisOffset((InputCode)val, (JoystickID)((type >> 8) & 0xF));
				}
				else
				{
					bool res = InputInstance.IsJoystickButtonPushed((InputCode)val, (JoystickID)((type >> 8) & 0xF));
					ret = (res ? 1.0f : 0.0f);
				}
			}
			else if (((type >> 4) & 0x7) == 0x1) //held
			{
				if (((type >> 7) & 0x1) == 0x1) //axis
				{
					ret = InputInstance.getJoystickAxisOffset((InputCode)val, (JoystickID)((type >> 8) & 0xF));
				}
				else
				{
					bool res = InputInstance.IsJoystickButtonPushed((InputCode)val, (JoystickID)((type >> 8) & 0xF));
					ret = (res ? 1.0f : 0.0f);
				}
			}
			else if (((type >> 4) & 0x7) == 0x2) //released - not implemented
			{
				bool res = false;
				ret = (res ? 1.0f : 0.0f);
			}
		}

		static float res_buff[1];
		res_buff[0] = ret;
		data->data = res_buff;
	}

	void GetGameObject(CallbackData* data)
	{
		GameObject* result = nullptr;

		void** ptrs = (void**)data->data;
		GameObject* parent = (GameObject*)ptrs[0];
		std::wstring obj_name = (wchar_t*)ptrs[1];

		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter;

		std::string obj_name_ = converter.to_bytes(obj_name);

		if (parent == nullptr)
		{
			result = Application::Get().getEngineLayer()->getRootObject();
		}
		else
		{
			result = (GameObject*)parent->getChild(obj_name_);
		}

		data->data = (void*)result;
	}

	extern "C" void __declspec(dllexport) ScriptCallback(CallbackData* data)
	{
		switch (data->type)
		{
		case CallbackType::HasComponent:
		{
			HasComponent(data);
			break;
		}
		case CallbackType::GetComponent:
		{
			GetComponent(data);
			break;
		}
		case CallbackType::SetComponent:
		{
			SetComponent(data);
			break;
		}
		case CallbackType::GetTransform:
		{
			GetTransform(data);
			break;
		}
		case CallbackType::SetTransform:
		{
			SetTransform(data);
			break;
		}
		case CallbackType::Log:
		{
			ScriptLog(data);
			break;
		}
		case CallbackType::InputCheck:
		{
			InputCheck(data);
			break;
		}
		case CallbackType::GetGameObject:
		{
			GetGameObject(data);
			break;
		}
		default:
			break;
		}
	}

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
		ExecuteFunction("BaseComponent", "Init", nullptr);
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

		if (obj->vtable == nullptr)
		{
			DEBUG_BREAK();
		}

		current_parent = this->parent;
		MonoObject* result = mono_runtime_invoke(method, obj, parameters, nullptr);
	}
};
