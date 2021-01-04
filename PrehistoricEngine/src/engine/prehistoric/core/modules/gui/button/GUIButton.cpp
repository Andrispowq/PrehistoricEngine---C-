#include "Includes.hpp"
#include "GUIButton.h"

#include "prehistoric/core/CoreEngine.h"

namespace Prehistoric
{
	GUIButton::GUIButton(Window* window, ResourceStorage* resourceStorage, Texture* texture, void* data, size_t dataSize, bool visible)
		: GUIElement(window, resourceStorage, texture, data, dataSize, visible)
	{
		type = GUIType::Button;
	}

	void GUIButton::PreUpdate(CoreEngine* engine)
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