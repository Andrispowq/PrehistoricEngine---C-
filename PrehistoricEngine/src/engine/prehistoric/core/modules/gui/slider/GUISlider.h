#ifndef GUI_SLIDER_H
#define GUI_SLIDER_H

#include "prehistoric/core/modules/gui/GUIElement.h"

namespace Prehistoric
{
	class GUISlider : public GUIElement
	{
	public:
		GUISlider(Window* window, AssembledAssetManager* manager, float minValue, float maxValue, Texture* texture = nullptr, void* data = nullptr, size_t dataSize = 0, bool visible = true);

		virtual void PreUpdate(CoreEngine* engine) override;

		float getProgress() const { return progress; }

		GUISlider(const GUISlider&) = default;

	private:
		float minValue;
		float maxValue;
		float progress;

		float cursorOldPositionX = -1;
	};
};

#endif