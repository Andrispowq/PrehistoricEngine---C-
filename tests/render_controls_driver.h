#pragma once
#include "tests/temporal_render_driver.h"
#ifdef PREHISTORIC_EDITOR_CONTROLS
#include "PrehistoricEditor/src/panels/SceneHierarchyPanel.h"
#endif

class RenderControlsDriver : public Prehistoric::Layer {
public:
    explicit RenderControlsDriver(Prehistoric::Scene *scene) : scene(scene) {
    }

    void Update(float) override { Prehistoric::Input::getInstance().setPauseRendering(false); }

    void ImGUIRender() override {
        using namespace Prehistoric;
        auto *renderer = Application::Get().getEngineLayer()->getRenderingEngine()->getRenderer();
        auto *vk = dynamic_cast<VKRenderer *>(renderer);
        auto *gl = dynamic_cast<GLRenderer *>(renderer);
        auto *settings = gl ? &gl->getVoxelPass()->GetSettings() : nullptr;
        const ImGuiKey keys[] = {
            ImGuiKey_G, ImGuiKey_V, ImGuiKey_T, ImGuiKey_H, ImGuiKey_M,
            ImGuiKey_J, ImGuiKey_RightBracket, ImGuiKey_LeftBracket, ImGuiKey_B, ImGuiKey_F8, ImGuiKey_C
        };
        auto value = [&](unsigned index) {
            switch (index) {
                case 0: return int(vk ? vk->voxelGI : settings->globalIllumination);
                case 1: return int(vk ? vk->voxelView : settings->debugView);
                case 2: return int(renderer->voxeliseTerrain);
                case 3: return int(vk ? vk->voxelHalfResolution : settings->halfResolution);
                case 4: return int(vk ? vk->voxelDirectional : settings->directionalMips);
                case 5: return int(vk ? vk->voxelSunShadows : settings->voxelSunShadows);
                case 6:
                case 7: return vk ? vk->voxelBounces : settings->traceDepth;
                case 8: return vk ? vk->voxelResolution : GLVoxelRendererPass::Resolutions[settings->resolutionIndex];
                case 10: return int(renderer->IsCameraInsideVoxelVolume());
                default:
                    auto *window = ImGui::FindWindowByName("Rendering comparison (F8)");
                    return int(window && window->Active);
            }
        };
        if (index == std::size(keys)) {
            if (scene->voxeliseTerrain != renderer->voxeliseTerrain)
                throw std::runtime_error(
                    "Terrain scene setting diverged");
#ifdef PREHISTORIC_EDITOR_CONTROLS
            if (placementFrames++ == 0) {
                if (vk) vk->voxelView = false;
                else settings->debugView = false;
                SceneHierarchyPanel panel(scene);
                panel.CreateProceduralObject(true);
                panel.CreateProceduralObject(false);
                if (!scene->getSceneRoot()->getChild("Terrain") || !scene->getSceneRoot()->getChild("Ocean"))
                    throw std::runtime_error("Procedural menu actions failed to create objects");
                WorldSerialiser(renderer->getWindow(), Application::Get().getEngineLayer()->getAssetManager())
                        .SerialiseWorldJSON("placed-procedural.json", scene);
            }
            if (placementFrames < 10) return;
            std::fprintf(stderr, "CONTROLS placement: terrain and ocean created, saved and rendered\n");
#endif
            std::fprintf(
                stderr, "CONTROLS PASS: rendering shortcuts, volume repositioning and terrain scene synchronisation\n");
            Application::Get().Stop();
            return;
        }
        if (phase == 0) {
            if (index == 10) {
                renderer->voxelFollowCamera = false;
                const auto volume = renderer->GetVoxelVolume();
                renderer->SetVoxelVolume({100000, 100000, 100000}, volume.w);
            }
            before = value(index);
            ImGui::GetIO().AddKeyEvent(keys[index], true);
        }
        if (phase == 1) ImGui::GetIO().AddKeyEvent(keys[index], false);
        if (phase == 3) {
            if (value(index) == before) throw std::runtime_error("Rendering shortcut did not change its setting");
            ++index;
        }
        phase = (phase + 1) % 4;
    }

private:
    Prehistoric::Scene *scene;
    unsigned index = 0, phase = 0;
    int before = 0;
    unsigned placementFrames = 0;
};