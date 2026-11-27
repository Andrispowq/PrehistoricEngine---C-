#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

#include "prehistoric/core/layers/Layer.h"

namespace Prehistoric {
    class Texture;
    class Renderer;
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

        static void DrawVoxelCascades(Renderer &renderer, unsigned nearResolution);

        void DrawImage(Texture *texture, float width, float height);

        void RenderPlatformWindows();

        void SetDarkThemeColors();

    private:
        bool showRenderingControls = false;
        std::unique_ptr<VKImGui> vulkan;
    };
};

#endif