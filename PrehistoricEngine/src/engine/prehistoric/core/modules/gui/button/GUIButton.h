#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "prehistoric/core/modules/gui/GUIElement.h"

namespace Prehistoric
{
	class GUIButton : public GUIElement
	{
	public:
		GUIButton(Window* window, AssembledAssetManager* manager, Vector3f colour = -1, void* data = nullptr, size_t dataSize = 0, bool visible = true);

		virtual void PreUpdate(CoreEngine* engine) override;

		GUIButton(const GUIButton&) = default;
	};
};

#endif