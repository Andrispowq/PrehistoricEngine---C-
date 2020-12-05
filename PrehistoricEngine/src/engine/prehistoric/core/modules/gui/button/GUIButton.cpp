#include "Includes.hpp"
#include "GUIButton.h"

#include "prehistoric/core/Engine.h"

namespace Prehistoric
{
	GUIButton::GUIButton(Window* window, AssembledAssetManager* manager, Texture* texture, void* data, size_t dataSize, bool visible)
		: GUIElement(window, manager, texture, data, dataSize, visible)
	{
		type = GUIType::Button;
	}

	void GUIButton::PreUpdate(Engine* engine)
	{
		bool* val = (bool*)data;

		if (InputInstance.IsButtonHeld(PR_MOUSE_LEFT_BUTTON))
		{
			if (inside(InputInstance.getCursorPosition()))
			{
				(*val) = true;
			}
			else
			{
				(*val) = false;
			}
		}
		else
		{
			(*val) = false;
		}
	}
};