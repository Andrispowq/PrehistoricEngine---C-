#ifndef GUI_SLIDER_H
#define GUI_SLIDER_H

#include "prehistoric/core/modules/gui/GUIElement.h"

namespace Prehistoric
{
	class GUISlider : public GUIElement
	{
	public:
		GUISlider(Window* window, AssembledAssetManager* manager, float minValue, float maxValue, Vector3f colour = -1, float* data = nullptr, size_t dataSize = 0, bool visible = true);
		~GUISlider();

		virtual void PreUpdate(CoreEngine* engine) override;
		virtual void PreRender(Renderer* renderer) override;

		float getMinValue() const { return minValue; }
		float getMaxValue() const { return maxValue; }
		float getProgress() const { return progress; }

		const Matrix4f& getSliderTransform() const { return slider_transform; }
		Vector3f getSliderColour() const { return colour + 0.1f; }

		void setMinValue(float minValue) { this->minValue = minValue; }
		void setMaxValue(float maxValue) { this->maxValue = maxValue; }
		void setProgress(float progress) { this->progress = progress; }

		GUISlider(const GUISlider&) = default;

	private:
		float minValue;
		float maxValue;
		float progress;

		Matrix4f slider_transform;
		Vector2f slider_position;
		Vector2f slider_scale;
		
		//This is the handle we can use to slide between values, sits on top of the standard GUI mesh
		static VertexBufferHandle slider_vbo;
		static PipelineHandle slider_pipeline;

		GUIElement* background;
		bool dragged = false;
		float cursorOldPositionX = -1;
	};
};

#endif