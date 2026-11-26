#pragma once
// Included only by the scratch executable built by temporal_render_smoke.py.
#include "prehistoric/application/Application.h"
#include "platform/vulkan/rendering/VKRenderer.h"
#include "platform/opengl/rendering/GLRenderer.h"
#include "prehistoric/core/scene/world/WorldSerialiser.h"
#include "imgui_internal.h"
#include "prehistoric/core/modules/water/Water.h"
#include <cstdio>
#include <stdexcept>
#ifdef __APPLE__
#include <dispatch/dispatch.h>
#endif

class TemporalRenderDriver : public Prehistoric::Layer {
public:
    explicit TemporalRenderDriver(Prehistoric::Scene *scene) : scene(scene) {
    }

    void Render() override {
        using namespace Prehistoric;
        auto &app = Application::Get();
        auto *renderer = app.getEngineLayer()->getRenderingEngine()->getRenderer();
        auto *window = static_cast<GLFWwindow *>(renderer->getWindow()->getWindowHandle());
        auto *camera = renderer->getCamera();
        ++frame;
        if (frame == 1) {
            originalView = camera->getViewMatrix();
            originalPosition = camera->getPosition();
            if (std::getenv("PREHISTORIC_TEMPORAL_NO_GI")) {
                if (auto *vk = dynamic_cast<VKRenderer *>(renderer))
                    vk->voxelGI = false;
                if (auto *gl = dynamic_cast<GLRenderer *>(renderer))
                    gl->getVoxelPass()->GetSettings().globalIllumination = false;
            }
            renderer->temporal.motionBlur = renderer->temporal.antialiasing = true;
            if (const char *debug = std::getenv("PREHISTORIC_TEMPORAL_DEBUG"))
                renderer->temporal.debug = std::atoi(debug);
            if (!scene->temporal.motionBlur || !scene->temporal.antialiasing)
                throw std::runtime_error("Temporal world settings were not loaded");
            WorldSerialiser(renderer->getWindow(), app.getEngineLayer()->getAssetManager())
                    .SerialiseWorldJSON("roundtrip.json", scene);
            auto loaded = std::make_unique<Scene>("roundtrip.json", renderer->getWindow(), camera,
                                                  app.getEngineLayer()->getAssetManager());
            if (!loaded->temporal.motionBlur || !loaded->temporal.antialiasing ||
                loaded->temporal.samples != scene->temporal.samples)
                throw std::runtime_error("Temporal settings round-trip failed");
            std::fprintf(stderr, "TEMPORAL settings round-trip passed\n");
            if (std::getenv("PREHISTORIC_TEMPORAL_WATER"))
                scene->getSceneRoot()->AddChild("Temporal water", new Water(renderer->getWindow(), camera,
                                                                            app.getEngineLayer()->getAssetManager(),
                                                                            originalPosition - Vector3f(0, 1, 0)));
        }
        if (frame == 8 && (!renderer->temporalHistory.valid || renderer->temporalHistory.frame < 2))
            throw std::runtime_error("Temporal history did not accumulate");
        if (frame >= 9 && frame < 16) {
            camera->setPosition(originalPosition + Vector3f(0.3f * float(frame - 8), 0, 0));
            camera->setViewMatrix(Matrix4f::View(camera->getForward(), camera->getUp()) *
                                  Matrix4f::Translation(camera->getPosition() * -1));
            camera->setViewProjectionMatrix(camera->getProjectionMatrix() * camera->getViewMatrix());
            auto *moving = scene->getSceneRoot()->getChild("Mirror Sphere");
            if (!moving)
                moving = scene->getSceneRoot()->getChild("Bar 4");
            if (auto *object = moving)
                object->Move({0.4f, 0, 0});
        }
        if (frame == 16 && !std::getenv("PREHISTORIC_TEMPORAL_BENCHMARK"))
            glfwSetWindowSize(window, 720, 420);
        if (std::getenv("PREHISTORIC_TEMPORAL_WATER")) {
            if (frame == 18) {
                renderer->setBloomEnabled(true);
                renderer->depthOfField.enabled = true;
            }
            if (frame == 20 || frame == 27) {
                camera->setPosition(originalPosition - Vector3f(0, frame == 20 ? 3 : 0, 0));
                camera->setViewMatrix(Matrix4f::View(camera->getForward(), camera->getUp()) *
                                      Matrix4f::Translation(camera->getPosition() * -1));
                camera->setViewProjectionMatrix(camera->getProjectionMatrix() * camera->getViewMatrix());
                renderer->ResetTemporalHistory();
            }
            if (frame == 26)
                renderer->depthOfField.enabled = false;
            if (frame == 32)
                renderer->setBloomEnabled(false);
        }
        if (frame == 20)
            renderer->temporal.antialiasing = false;
        if (frame == 24)
            renderer->temporal.antialiasing = true;
#ifdef __APPLE__
        if (frame == 26 && !std::getenv("PREHISTORIC_TEMPORAL_BENCHMARK")) {
            dispatch_after_f(dispatch_time(DISPATCH_TIME_NOW, 300 * NSEC_PER_MSEC), dispatch_get_main_queue(), window,
                             [](void *target) {
                                 glfwRestoreWindow(static_cast<GLFWwindow *>(target));
                                 std::fprintf(stderr, "TEMPORAL restored after minimization\n");
                             });
            glfwIconifyWindow(window);
        }
#endif
        if (frame == 28)
            renderer->temporal.motionBlur = renderer->temporal.antialiasing = false;
        if (frame == 30)
            renderer->temporal.motionBlur = renderer->temporal.antialiasing = true;
        if (frame == 34)
            renderer->RequestEnvironmentBake(true);
        if (frame == 35) renderer->setWireframeMode(true);
        if (frame == 36) {
            renderer->setWireframeMode(false);
            renderer->ResetTemporalHistory();
        }
        if (frame >= 33 && frame <= 40)
            for (int i = 0; i < 4; ++i)
                if (renderer->temporalMilliseconds[i] >= 0) timings[i].push_back(renderer->temporalMilliseconds[i]);
        if (frame == 40) {
            if (auto *vk = dynamic_cast<VKRenderer *>(renderer)) {
                auto *swapchain = static_cast<VKSwapchain *>(vk->getWindow()->getSwapchain());
                auto extent = swapchain->getSwapchainExtent();
                std::fprintf(stderr, "TEMPORAL render extent: %u x %u; MSAA: %u\n", extent.width, extent.height,
                             unsigned(swapchain->getSceneSamples()));
            }
            for (int i = 0; i < 4; ++i) {
                auto sorted = timings[i];
                std::sort(sorted.begin(), sorted.end());
                if (!sorted.empty())
                    std::fprintf(stderr, "TEMPORAL stage %d: median %.4f ms; range %.4f-%.4f; observations %zu\n",
                                 i, sorted[sorted.size() / 2], sorted.front(), sorted.back(), sorted.size());
            }
            std::fprintf(stderr, "TEMPORAL target bytes: %llu; history frames: %llu\n",
                         static_cast<unsigned long long>(renderer->temporalBytes),
                         static_cast<unsigned long long>(renderer->temporalHistory.frame));
        }
        if (frame == 42) {
            std::fprintf(stderr, "TEMPORAL PASS: 41 frames; camera/object motion, resize, toggles, rebake and reset\n");
            app.Stop();
        }
    }

    void ImGUIRender() override {
        // Exercise the editor's scene image in a detached platform window, then dock it again.
        if (std::getenv("PREHISTORIC_TEMPORAL_BENCHMARK")) return;
        auto *panel = ImGui::FindWindowByName("Viewport");
        if (!panel)
            return;
        if (frame == 18) {
            viewportDock = panel->DockId;
            ImGui::DockContextQueueUndockWindow(ImGui::GetCurrentContext(), panel);
        }
        if (frame == 22 && viewportDock) {
            ImGui::SetWindowDock(panel, viewportDock, ImGuiCond_Always);
            ImGui::SetWindowFocus("Viewport");
        }
    }

private:
    Prehistoric::Scene *scene;
    Prehistoric::Matrix4f originalView;
    Prehistoric::Vector3f originalPosition;
    int frame = 0;
    ImGuiID viewportDock = 0;
    std::vector<double> timings[4];
};