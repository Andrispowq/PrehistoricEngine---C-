#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

#include "prehistoric/core/layers/Layer.h"

namespace Prehistoric {
    class Texture;
    class VKImGui;

    class ImGuiLayer : public Layer {
    public:
        ImGuiLayer();

        ~ImGuiLayer();

        virtual void OnAttach() override;

        virtual void OnDetach() override;

        void Begin();

        void End();

        void Draw();

        void DrawImage(Texture *texture, float width, float height);

        void RenderPlatformWindows();

        void SetDarkThemeColors();

    private:
        std::unique_ptr<VKImGui> vulkan;
    };
};

#endif