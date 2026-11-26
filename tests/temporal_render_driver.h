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

    // The scratch runner must make progress when launched behind the editor/Codex window.
    // This affects only the test executable; production focus behavior is unchanged.
    void Update(float) override {
        Prehistoric::Input::getInstance().setPauseRendering(false);
    }

    void Render() override {
        using namespace Prehistoric;
        auto &app = Application::Get();
        auto *renderer = app.getEngineLayer()->getRenderingEngine()->getRenderer();
        auto *window = static_cast<GLFWwindow *>(renderer->getWindow()->getWindowHandle());
        auto *camera = renderer->getCamera();
        if (!renderer->CanRenderFrame()) return;
        ++frame;
        const auto extent = renderer->GetRenderExtent();
        const auto &history = renderer->temporalHistory;
        if (history.valid && history.frame && (history.jitter.x != 0 || history.jitter.y != 0)) {
            const unsigned index = unsigned((history.frame - 1) % 8) + 1;
            if (std::abs(history.jitter.x * extent.width - (TemporalHistory::Halton(index, 2) - 0.5f)) > 1e-4f ||
                std::abs(history.jitter.y * extent.height - (TemporalHistory::Halton(index, 3) - 0.5f)) > 1e-4f)
                throw std::runtime_error("Temporal jitter is not normalized to the render extent");
        }
        if (frame == 2 && (!history.valid || history.frame < 2)) {
            if (++warmupFrames > 120) throw std::runtime_error("Temporal history did not accumulate after warm-up");
            --frame;
            return;
        }
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
        if (frame == 16)
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
        if (frame == 26) {
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
        if (frame == 40) {
            if (auto *vk = dynamic_cast<VKRenderer *>(renderer)) {
                auto *swapchain = static_cast<VKSwapchain *>(vk->getWindow()->getSwapchain());
                auto extent = swapchain->getSwapchainExtent();
                std::fprintf(stderr, "TEMPORAL render extent: %u x %u; MSAA: %u\n", extent.width, extent.height,
                             unsigned(swapchain->getSceneSamples()));
            }
            std::fprintf(stderr, "TEMPORAL target bytes: %llu; history frames: %llu\n",
                         static_cast<unsigned long long>(renderer->temporalBytes),
                         static_cast<unsigned long long>(renderer->temporalHistory.frame));
        }
        if (frame == 43 || frame == 49 || frame == 55) {
            const auto supported = renderer->GetSupportedSampleCounts();
            unsigned multisample = 1;
            for (unsigned samples: supported) if (samples <= 4) multisample = samples;
            expectedSamples = frame == 49 ? 1 : multisample;
            sampleChangeExpected = expectedSamples != renderer->GetSampleCount();
            if (renderer->RequestSampleCount(3)) throw std::runtime_error("Unsupported sample count accepted");
            if (!renderer->RequestSampleCount(expectedSamples)) throw std::runtime_error(
                "Supported sample count rejected");
        }
        if (frame == 44 || frame == 50 || frame == 56) {
            if (renderer->GetSampleCount() != expectedSamples) throw std::runtime_error(
                "Live MSAA change was not applied");
            if (sampleChangeExpected && renderer->temporalHistory.valid) throw std::runtime_error(
                "Live MSAA change retained temporal history");
            std::fprintf(stderr, "TEMPORAL live MSAA: %u samples; %s\n", expectedSamples,
                         sampleChangeExpected ? "history invalidated" : "unchanged request");
        }
        scene->temporal = renderer->temporal;
        if (frame == 70) {
            std::fprintf(
                stderr,
                "TEMPORAL PASS: lifecycle; camera/object motion, resize, toggles, rebake, reset and live MSAA\n");
            app.Stop();
        }
    }

    void ImGUIRender() override {
        // Exercise the editor's scene image in a detached platform window, then dock it again.
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
    unsigned warmupFrames = 0, expectedSamples = 1;
    bool sampleChangeExpected = false;
};