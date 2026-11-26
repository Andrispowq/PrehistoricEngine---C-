#pragma once
// Scratch application only: no lifecycle mutations during measured phases.
#include "prehistoric/application/Application.h"
#include "platform/vulkan/rendering/VKRenderer.h"
#include "platform/opengl/rendering/GLRenderer.h"
#include <chrono>
#include <cstdio>

class TemporalRenderDriver : public Prehistoric::Layer {
public:
    explicit TemporalRenderDriver(Prehistoric::Scene *) {
        if (const char *value = std::getenv("PREHISTORIC_BENCHMARK_WARMUP")) warmup = unsigned(std::atoi(value));
        if (const char *value = std::getenv("PREHISTORIC_BENCHMARK_FRAMES")) measured = unsigned(std::atoi(value));
        moving = std::getenv("PREHISTORIC_BENCHMARK_MOVING") != nullptr;
    }

    void Update(float) override {
        Prehistoric::Input::getInstance().setPauseRendering(false);
    }

    void Render() override {
        using namespace Prehistoric;
        auto &app = Application::Get();
        auto *renderer = app.getEngineLayer()->getRenderingEngine()->getRenderer();
        if (!renderer->CanRenderFrame()) return;
        auto *camera = renderer->getCamera();
        const auto now = std::chrono::steady_clock::now();
        // One extra callback observes the last measured frame before changing modes.
        const unsigned length = warmup + measured + 1;
        const unsigned mode = frame / length, local = frame % length;
        if (mode == 4) {
            std::fprintf(stderr, "TEMPORAL PASS: steady benchmark; four effect modes\n");
            app.Stop();
            return;
        }
        if (frame == 0) {
            originalPosition = camera->getPosition();
            if (std::getenv("PREHISTORIC_TEMPORAL_NO_GI")) {
                if (auto *vk = dynamic_cast<VKRenderer *>(renderer)) vk->voxelGI = false;
                if (auto *gl = dynamic_cast<GLRenderer *>(renderer))
                    gl->getVoxelPass()->GetSettings().globalIllumination = false;
            }
        }
        if (local == 0) {
            frameTimes.clear();
            for (auto &values: timings) values.clear();
            renderer->ResetTemporalHistory();
            firstMeasuredSubmission = renderer->submittedFrames + warmup + 1;
        }
        if (local > warmup) {
            frameTimes.push_back(std::chrono::duration<double, std::milli>(now - previous).count());
            const auto &sample = renderer->temporalTimings;
            if (sample.frame >= firstMeasuredSubmission && sample.frame > lastTimingFrame && sample.mode == mode) {
                lastTimingFrame = sample.frame;
                for (unsigned i = 0; i < 4; ++i) {
                    if ((i == 2 && !(mode & 1)) || (i == 3 && !(mode & 2))) continue;
                    timings[i].push_back(sample.milliseconds[i]);
                }
            }
        }
        previous = now;
        renderer->temporal.antialiasing = (mode & 1) != 0;
        renderer->temporal.motionBlur = (mode & 2) != 0;
        renderer->temporal.debug = 0;
        const float offset = moving ? 2.0f * std::sin(float(local) * 0.06f) : 0;
        camera->setPosition(originalPosition + Vector3f(offset, 0, 0));
        camera->setViewMatrix(Matrix4f::View(camera->getForward(), camera->getUp()) *
                              Matrix4f::Translation(camera->getPosition() * -1));
        camera->setViewProjectionMatrix(camera->getProjectionMatrix() * camera->getViewMatrix());
        if (local + 1 == length) {
            const auto extent = renderer->GetRenderExtent();
            std::fprintf(stderr, "TEMPORAL BASELINE mode=%u camera=%s extent=%ux%u MSAA=%u target_bytes=%llu\n",
                         mode, moving ? "moving" : "fixed", extent.width, extent.height, renderer->GetSampleCount(),
                         static_cast<unsigned long long>(renderer->temporalBytes));
            Print("frame interval", frameTimes);
            const char *names[] = {"motion geometry", "motion resolve", "TAA", "blur"};
            for (unsigned i = 0; i < 4; ++i) Print(names[i], timings[i]);
        }
        ++frame;
    }

private:
    static void Print(const char *name, std::vector<double> values) {
        if (values.empty()) {
            std::fprintf(stderr, "TEMPORAL %s: unavailable\n", name);
            return;
        }
        std::sort(values.begin(), values.end());
        std::fprintf(stderr, "TEMPORAL %s: median=%.4f p95=%.4f ms observations=%zu\n", name,
                     values[values.size() / 2], values[std::min(values.size() - 1, values.size() * 95 / 100)],
                     values.size());
    }

    unsigned frame = 0, warmup = 60, measured = 120;
    bool moving = false;
    uint64_t firstMeasuredSubmission = 0, lastTimingFrame = 0;
    Prehistoric::Vector3f originalPosition;
    std::chrono::steady_clock::time_point previous;
    std::vector<double> frameTimes, timings[4];
};