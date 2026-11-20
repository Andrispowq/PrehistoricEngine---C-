#include "Includes.hpp"
#include "InternalCalls.h"

#include "prehistoric/application/Application.h"

#define PR_ADD_INTERNAL_CALL(name) mono_add_internal_call("Prehistoric.InternalCalls::" #name, name)

namespace Prehistoric
{
	extern GameObject* current_parent;

	static bool HasComponent(MonoString* name)
	{
		bool result = false;

		char* cStr = mono_string_to_utf8(name);
		std::string comp_name = cStr;
		if (comp_name == "RenderableComponent")
		{
			result = current_parent->HasComponent<RenderableComponent>();
		}
		else if (comp_name == "RendererComponent")
		{
			result = current_parent->HasComponent<RendererComponent>();
		}
		else if (comp_name == "Light")
		{
			result = current_parent->HasComponent<Light>();
		}
		else if (comp_name == "PhysicsComponent")
		{
			result = current_parent->HasComponent<PhysicsComponent>();
		}
		else if (comp_name == "AudioComponent")
		{
			result = current_parent->HasComponent<AudioComponent>();
		}
		else if (comp_name == "ScriptComponent")
		{
			result = current_parent->HasComponent<ScriptComponent>();
		}
		else if (comp_name == "CameraComponent")
		{
			result = current_parent->HasComponent<CameraComponent>();
		}

		return result;
	}

	static void GetComponent(MonoString* name, void* data)
	{
		char* cStr = mono_string_to_utf8(name);
		std::string comp_name = cStr;
		if (comp_name == "RenderableComponent")
		{
			data = (void*)current_parent->GetComponent<RenderableComponent>();
		}
		else if (comp_name == "RendererComponent")
		{
			RendererComponent* comp = current_parent->GetComponent<RendererComponent>();
			Material* mat = comp->getMaterial();
			Vector3f colour = mat->getVector3f(COLOUR);
			Vector4f mrot = mat->getVector4f(MROT);

			float* fdata = (float*)data;
			fdata[0] = colour.r;
			fdata[1] = colour.g;
			fdata[2] = colour.b;
			fdata[3] = mrot.r;
			fdata[4] = mrot.g;
			fdata[5] = mrot.b;

			//data = (void*)fdata;
		}
		else if (comp_name == "Light")
		{
			data = (void*)current_parent->GetComponent<Light>();
		}
		else if (comp_name == "PhysicsComponent")
		{
			data = (void*)current_parent->GetComponent<PhysicsComponent>();
		}
		else if (comp_name == "AudioComponent")
		{
			data = (void*)current_parent->GetComponent<AudioComponent>();
		}
		else if (comp_name == "ScriptComponent")
		{
			data = (void*)current_parent->GetComponent<ScriptComponent>();
		}
		else if (comp_name == "CameraComponent")
		{
			CameraComponent* comp = current_parent->GetComponent<CameraComponent>();
			Camera* cam = comp->getCamera();

			Vector3f position = cam->getPosition();
			Vector3f forward = cam->getForward();
			Vector3f up = cam->getUp();

			float* fdata = (float*)data;
			fdata[0] = position.r;
			fdata[1] = position.g;
			fdata[2] = position.b;
			fdata[3] = forward.r;
			fdata[4] = forward.g;
			fdata[5] = forward.b;
			fdata[6] = up.r;
			fdata[7] = up.g;
			fdata[8] = up.b;

			//result = (void*)transfer;
		}
	}

	static void SetComponent(MonoString* name, void* data)
	{
		void* result = nullptr;

		char* cStr = mono_string_to_utf8(name);
		std::string comp_name = cStr;
		if (comp_name == "RenderableComponent")
		{
			RenderableComponent* comp = current_parent->GetComponent<RenderableComponent>();
		}
		else if (comp_name == "RendererComponent")
		{
			RendererComponent* comp = current_parent->GetComponent<RendererComponent>();
			Material* mat = comp->getMaterial();

			float* fdata = (float*)data;
			Vector3f colour = { fdata[0], fdata[1], fdata[2] };
			Vector4f mrot = { fdata[3], fdata[4], fdata[5], 0.0f };

			if (mat->getVector3f(COLOUR) == Vector3f(-1)) { mat->addVector3f(COLOUR, colour); }
			if (mat->getVector4f(MROT) == Vector4f(-1)) { mat->addVector4f(MROT, mrot); }

			mat->getVector3fs()[COLOUR] = colour;
			mat->getVector4fs()[MROT] = mrot;

			result = nullptr;
		}
		else if (comp_name == "Light")
		{
			Light* comp = current_parent->GetComponent<Light>();
		}
		else if (comp_name == "PhysicsComponent")
		{
			PhysicsComponent* comp = current_parent->GetComponent<PhysicsComponent>();
		}
		else if (comp_name == "AudioComponent")
		{
			AudioComponent* comp = current_parent->GetComponent<AudioComponent>();
		}
		else if (comp_name == "ScriptComponent")
		{
			ScriptComponent* comp = current_parent->GetComponent<ScriptComponent>();
		}
		else if (comp_name == "CameraComponent")
		{
			CameraComponent* comp = current_parent->GetComponent<CameraComponent>();
			Camera* cam = comp->getCamera();

			float* fdata = (float*)data;
			Vector3f position = { fdata[0], fdata[1], fdata[2] };
			Vector3f forward = { fdata[3], fdata[4], fdata[5] };
			Vector3f up = { fdata[6], fdata[7], fdata[8] };

			cam->setPosition(position);
			cam->setForward(forward);
			cam->setUp(up);
			cam->Update(nullptr, 0.0f);

			result = nullptr;
		}
	}

	static void GetTransform(float* buffer, void* parent)
	{
		GameObject* obj = current_parent;
		if (parent != nullptr)
		{
			obj = (GameObject*)parent;
		}

		Transform& transform = obj->getWorldTransform();

		buffer[0] = transform.getPosition().x;
		buffer[1] = transform.getPosition().y;
		buffer[2] = transform.getPosition().z;
		buffer[3] = transform.getRotation().x;
		buffer[4] = transform.getRotation().y;
		buffer[5] = transform.getRotation().z;
		buffer[6] = transform.getScaling().x;
		buffer[7] = transform.getScaling().y;
		buffer[8] = transform.getScaling().z;
	}

	void SetTransform(float* buffer)
	{
		Transform& transform = current_parent->getWorldTransform();

		transform.setPosition({ buffer[0], buffer[1], buffer[2] });
		transform.setRotation({ buffer[3], buffer[4], buffer[5] });
		transform.setScaling({ buffer[6], buffer[7], buffer[8] });
	}

	static void ScriptLog(MonoString* string)
	{
		char* cStr = mono_string_to_utf8(string);

		PR_LOG_MESSAGE("[%s - ScriptComponent]: %s\n", current_parent->getName().c_str(), cStr);
	}

	static float InputCheck(int type, int val)
	{
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

		return ret;
	}

	static GameObject* GetSceneRoot()
	{
		GameObject* root = Application::Get().getEngineLayer()->getSceneRoot();
		return root;
	}

	static GameObject* GetGameObject(MonoString* name, void* ptr_parent)
	{
		GameObject* result = nullptr;

		GameObject* parent = (GameObject*)ptr_parent;
		char* cStr = mono_string_to_utf8(name);
		std::string obj_name = cStr;
		if (parent == nullptr)
		{
			result = nullptr;
		}
		else
		{
			result = (GameObject*)parent->getChild(obj_name);
		}

		return result;
	}

	void InternalCalls::RegisterInternalCalls()
	{
		PR_ADD_INTERNAL_CALL(HasComponent);
		PR_ADD_INTERNAL_CALL(GetComponent);
		PR_ADD_INTERNAL_CALL(SetComponent);

		PR_ADD_INTERNAL_CALL(GetTransform);
		PR_ADD_INTERNAL_CALL(SetTransform);

		PR_ADD_INTERNAL_CALL(ScriptLog);
		PR_ADD_INTERNAL_CALL(InputCheck);

		PR_ADD_INTERNAL_CALL(GetSceneRoot);
		PR_ADD_INTERNAL_CALL(GetGameObject);
	}
}