#pragma once
#include "tests/temporal_render_driver.h"

// Capture settled GI-off, GI-on and occupancy frames in the real application scene.
class VoxelVisualDriver : public Prehistoric::Layer {
public:
    explicit VoxelVisualDriver(Prehistoric::Scene *) {
    }

    void Update(float) override { Prehistoric::Input::getInstance().setPauseRendering(false); }

    void ImGUIRender() override {
        using namespace Prehistoric;
        auto *renderer = Application::Get().getEngineLayer()->getRenderingEngine()->getRenderer();
        auto *vk = dynamic_cast<VKRenderer *>(renderer);
        auto *gl = dynamic_cast<GLRenderer *>(renderer);
        auto *settings = gl ? &gl->getVoxelPass()->GetSettings() : nullptr;
        auto &gi = vk ? vk->voxelGI : settings->globalIllumination;
        auto &view = vk ? vk->voxelView : settings->debugView;
        ++frame;
        if (frame == 1) {
            if (const char *width = std::getenv("PREHISTORIC_TEST_VOXEL_WIDTH")) {
                renderer->SetVoxelVolume(renderer->getCamera()->getPosition(), std::stof(width));
                renderer->FrameVoxelVolume();
            }
            if (!renderer->IsCameraInsideVoxelVolume()) throw std::runtime_error(
                "Startup camera outside voxel coverage");
            renderer->voxeliseTerrain = true;
            renderer->temporal = {};
            renderer->screenSpaceReflections = false;
            renderer->ambientOcclusion.enabled = false;
            renderer->setBloomEnabled(false);
            gi = view = false;
        }
        if (frame == 6) gi = true;
        if (frame == 14) view = true;
        if (frame == 22) {
            std::fprintf(
                stderr, "VOXEL VISUAL PASS: GI off/on and occupancy frames captured; startup camera inside volume\n");
            Application::Get().Stop();
        }
    }

private:
    unsigned frame = 0;
};