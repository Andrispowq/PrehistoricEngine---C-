#pragma once

#include "prehistoric/core/util/math/Math.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <unordered_map>

namespace Prehistoric {
    struct RenderExtent {
        unsigned width, height;
    };

    struct TemporalTimings {
        uint64_t frame = 0;
        unsigned mode = 0;
        std::array<double, 4> milliseconds{-1, -1, -1, -1};

        bool Publish(uint64_t submittedFrame, unsigned effects, const std::array<double, 4> &values) {
            if (submittedFrame <= frame) return false;
            frame = submittedFrame;
            mode = effects;
            milliseconds = values;
            return true;
        }
    };

    struct TemporalSettings {
        bool motionBlur = false;
        bool antialiasing = false;
        float shutterAngle = 180.0f;
        float maxBlurPixels = 32.0f;
        int samples = 16;
        int debug = 0; // 0: output, 1: velocity, 2: validity, 3: rejection, 4: history weight.
        bool active() const { return motionBlur || antialiasing || debug != 0; }
    };

    // Only the submitted main view advances this state. Capture views never own history.
    class TemporalHistory {
    public:
        Matrix4f projection = Matrix4f::Identity(), view = Matrix4f::Identity(), current = Matrix4f::Identity(),
                previous = Matrix4f::Identity(), previousView = Matrix4f::Identity();
        Vector2f jitter{0}, previousJitter{0};
        bool valid = false;
        uint64_t frame = 0;
        std::unordered_map<uint64_t, Matrix4f> models, pendingModels;

        static float Halton(unsigned index, unsigned base) {
            float value = 0, fraction = 1;
            while (index) {
                fraction /= float(base);
                value += fraction * float(index % base);
                index /= base;
            }
            return value;
        }

        void Reset() {
            valid = false;
            prepared = false;
            frame = 0;
            models.clear();
            pendingModels.clear();
        }

        void Begin(const Matrix4f &p, const Matrix4f &v, unsigned width, unsigned height, bool taa) {
            auto now = std::chrono::steady_clock::now();
            if (width != lastWidth || height != lastHeight || taa != lastTAA ||
                std::chrono::duration<double>(now - submitted).count() > 0.25)
                Reset();
            if (valid) {
                for (int i = 0; i < 16; ++i)
                    if (std::abs(p.m[i] - projection.m[i]) > 0.0001f) {
                        Reset();
                        break;
                    }
                // Explicit reset is available for cuts; this catches abrupt editor camera jumps too.
                auto delta = v * previousView.Invert();
                if (delta.m[12] * delta.m[12] + delta.m[13] * delta.m[13] + delta.m[14] * delta.m[14] > 2500 ||
                    delta.m[0] + delta.m[5] + delta.m[10] < 1.0f)
                    Reset();
            }
            projection = p;
            view = v;
            const auto product = p * v;
            current = product;
            if (!valid) {
                previous = current;
                previousView = view;
            }
            lastWidth = width;
            lastHeight = height;
            lastTAA = taa;
            jitter = taa
                         ? Vector2f((Halton(unsigned(frame % 8) + 1, 2) - 0.5f) / std::max(1u, width),
                                    (Halton(unsigned(frame % 8) + 1, 3) - 0.5f) / std::max(1u, height))
                         : Vector2f(0);
            pendingModels.clear();
            prepared = true;
        }

        Matrix4f JitteredProjection() const {
            auto result = projection;
            for (int column = 0; column < 4; ++column) {
                result.m[column * 4] += 2 * jitter.x * projection.m[column * 4 + 3];
                result.m[column * 4 + 1] += 2 * jitter.y * projection.m[column * 4 + 3];
            }
            return result;
        }

        bool CanReproject(uint64_t identity, const Matrix4f &model) const {
            auto found = models.find(identity);
            if (!valid || found == models.end())
                return false;
            float distance = 0;
            for (int i = 12; i < 15; ++i) {
                const float delta = model.m[i] - found->second.m[i];
                distance += delta * delta;
            }
            return std::isfinite(distance) && distance <= 2500;
        }

        void Commit() {
            if (!prepared) return;
            prepared = false;
            previous = current;
            previousJitter = jitter;
            previousView = view;
            models.swap(pendingModels);
            pendingModels.clear();
            valid = true;
            ++frame;
            submitted = std::chrono::steady_clock::now();
        }

    private:
        unsigned lastWidth = 0, lastHeight = 0;
        bool lastTAA = false;
        bool prepared = false;
        std::chrono::steady_clock::time_point submitted{};
    };
} // namespace Prehistoric