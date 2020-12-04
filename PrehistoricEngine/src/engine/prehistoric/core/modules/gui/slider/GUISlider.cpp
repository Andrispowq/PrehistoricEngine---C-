#include "engine/prehistoric/core/util/Includes.hpp"
#include "GUISlider.h"

GUISlider::GUISlider(Window* window, AssembledAssetManager* manager, float minValue, float maxValue, Texture* texture, void* data, size_t dataSize, bool visible)
	: GUIElement(window, manager, texture, data, dataSize, visible), minValue(minValue), maxValue(maxValue), progress(0.5f)
{
	type = GUIType::Slider;
}

void GUISlider::PreUpdate(Engine* engine)
{
	float* _data = (float*)data;

	if (InputInstance.IsButtonHeld(PR_MOUSE_BUTTON_LEFT) && inside(InputInstance.getCursorPosition()))
	{
		if (cursorOldPositionX == -1)
		{
			cursorOldPositionX = InputInstance.getCursorPosition().x;
		}
		else
		{
			float newPosition = InputInstance.getCursorPosition().x;

			float diff = newPosition - cursorOldPositionX;
			
			float dX = std::min(diff / (worldTransform.getScaling().x * window->getWidth() * 2), 1.0f);
			progress += dX;
			progress = std::max(std::min(progress, 1.0f), 0.0f);
			cursorOldPositionX = newPosition;
		}
	}
	else
	{
		cursorOldPositionX = -1;
	}

	*_data = minValue + (maxValue - minValue) * progress;
}
