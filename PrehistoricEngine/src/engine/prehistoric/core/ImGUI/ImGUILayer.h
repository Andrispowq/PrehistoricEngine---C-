#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

#include "prehistoric/core/layers/Layer.h"

namespace Prehistoric
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void Begin();
		void End();

		void SetDarkThemeColors();
	};
};

#endif