#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "prehistoric/common/rendering/Temporal.h"
#include "prehistoric/core/util/math/MathT.h"

using namespace Prehistoric;
using Catch::Matchers::WithinAbs;

namespace {
    Vector2f uv(const Matrix4f &transform, const Vector4f &vertex) {
        auto clip = transform * vertex;
        return {0.5f + 0.5f * clip.x / clip.w, 0.5f + 0.5f * clip.y / clip.w};
    }
} // namespace

TEST_CASE (

"Temporal reprojection includes camera and object transforms"
,
"[temporal]"
)
{
    const auto projection = Matrix4f::PerspectiveProjection(70, 1.5f, 0.1f, 1000);
    const auto previousModel = Matrix4f::Translation({0, 0, -10});
    const auto currentModel = Matrix4f::Translation({2, 0, -10});
    const auto currentView = Matrix4f::Translation({-1, 0, 0});
    const Vector4f vertex(0, 0, 0, 1);
    auto before = uv(projection * previousModel, vertex);
    auto now = uv(projection * currentView * currentModel, vertex);
    auto motion = before - now;
    CHECK(motion.x < 0);
    CHECK_THAT(now.x + motion.x, WithinAbs(before.x, 1e-6));
    CHECK_THAT(now.y + motion.y, WithinAbs(before.y, 1e-6));
    // Camera and object translating together cancel exactly.
    auto followed = uv(projection * Matrix4f::Translation({-2, 0, 0}) * currentModel, vertex);
    CHECK_THAT(followed.x, WithinAbs(before.x, 1e-6));
}

TEST_CASE (

"Jitter shifts raster coordinates but never physical velocity"
,
"[temporal]"
)
{
    TemporalHistory history;
    auto projection = Matrix4f::PerspectiveProjection(70, 1.5f, 0.1f, 1000);
    for (bool vulkan : {false, true})
    {
        if (vulkan)
            projection.m[5] *= -1;
        history.Reset();
        history.Begin(projection, Matrix4f::Identity(), 1200, 800, true);
        const Vector4f vertex(1, 2, -10, 1);
        const auto stable = uv(history.current, vertex);
        const auto raster = uv(history.JitteredProjection(), vertex);
        CHECK_THAT(raster.x - stable.x, WithinAbs(history.jitter.x, 1e-6));
        CHECK_THAT(raster.y - stable.y, WithinAbs(history.jitter.y, 1e-6));
        history.Commit();
        history.Begin(projection, Matrix4f::Identity(), 1200, 800, true);
        auto previous = uv(history.previous, vertex);
        auto current = uv(history.current, vertex);
        CHECK_THAT(previous.x - current.x, WithinAbs(0, 1e-6));
        CHECK_THAT(previous.y - current.y, WithinAbs(0, 1e-6));
    }
}

TEST_CASE (

"Only committed frames advance temporal state"
,
"[temporal]"
)
{
    TemporalHistory history;
    const auto projection = Matrix4f::PerspectiveProjection(70, 1.5f, 0.1f, 1000);
    history.Begin(projection, Matrix4f::Identity(), 1200, 800, true);
    CHECK_FALSE(history.valid);
    CHECK(history.frame == 0);
    history.pendingModels.emplace(42, Matrix4f::Identity());
    CHECK(history.models.empty());
    history.Commit();
    CHECK(history.valid);
    CHECK(history.frame == 1);
    CHECK(history.models.count(42) == 1);
    history.Begin(projection, Matrix4f::Identity(), 1200, 800, true);
    CHECK(history.frame == 1);
    CHECK(history.models.count(42) == 1);
    history.Commit();
    CHECK(history.models.empty()); // Objects absent from this submission cannot retain stale state.
    history.Begin(projection, Matrix4f::Identity(), 600, 400, true);
    CHECK_FALSE(history.valid);
    CHECK(history.frame == 0);
    history.Commit();
    history.Begin(projection, Matrix4f::Identity(), 600, 400, false);
    CHECK_FALSE(history.valid);
    CHECK(history.jitter.x == 0);
    history.Reset();
    CHECK(history.models.empty());
}

TEST_CASE (

"Previous surface depth follows the previous object pose"
,
"[temporal]"
)
{
    const auto projection = Matrix4f::PerspectiveProjection(70, 1.5f, 0.1f, 1000);
    auto previous = projection * Matrix4f::Translation({0, 0, -10}) * Vector4f(0, 0, 0, 1);
    auto current = projection * Matrix4f::Translation({0, 0, -5}) * Vector4f(0, 0, 0, 1);
    CHECK_THAT(previous.w, WithinAbs(10, 1e-6));
    CHECK_THAT(current.w, WithinAbs(5, 1e-6));
    // Comparing stored depth 10 against current depth 5 would incorrectly reject this surface.
    CHECK(std::abs(previous.w - 10) < std::max(0.02f, previous.w * 0.01f));
    CHECK(std::abs(current.w - 10) > std::max(0.02f, current.w * 0.01f));
}

TEST_CASE (

"New identities and teleports reject object history"
,
"[temporal]"
)
{
    TemporalHistory history;
    const auto model = Matrix4f::Translation({0, 0, -10});
    history.Begin(Matrix4f::Identity(), Matrix4f::Identity(), 100, 100, false);
    history.pendingModels.emplace(7, model);
    history.Commit();
    CHECK(history.CanReproject(7, Matrix4f::Translation({1, 0, -10})));
    CHECK_FALSE(history.CanReproject(8, model));
    CHECK_FALSE(history.CanReproject(7, Matrix4f::Translation({100, 0, -10})));
    history.Reset();
    CHECK_FALSE(history.CanReproject(7, model));
}

TEST_CASE (

"Reset after recording cannot publish stale history"
,
"[temporal]"
)
{
    TemporalHistory history;
    history.Begin(Matrix4f::Identity(), Matrix4f::Identity(), 100, 100, true);
    history.Reset(); // For example, an OpenGL editor UI reset after scene rendering.
    history.Commit();
    CHECK_FALSE(history.valid);
    CHECK(history.frame == 0);
    history.Begin(Matrix4f::Identity(), Matrix4f::Identity(), 100, 100, true);
    history.Commit();
    history.Commit(); // Submission is consumed exactly once.
    CHECK(history.frame == 1);
}

TEST_CASE (

"Long pauses invalidate temporal history on resume"
,
"[temporal]"
)
{
    TemporalHistory history;
    history.Begin(Matrix4f::Identity(), Matrix4f::Identity(), 100, 100, true);
    history.Commit();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    history.Begin(Matrix4f::Identity(), Matrix4f::Identity(), 100, 100, true);
    CHECK_FALSE(history.valid);
    CHECK(history.frame == 0);
}
TEST_CASE (
"Temporal jitter keeps its pixel footprint across render scales"
,
"[temporal]"
)
{
    const auto projection = Matrix4f::PerspectiveProjection(70, 16.0f / 9, 0.1f, 1000);
    TemporalHistory standard, retina;
    for (unsigned frame = 0; frame < 16; ++frame)
    {
        standard.Begin(projection, Matrix4f::Identity(), 960, 540, true);
        retina.Begin(projection, Matrix4f::Identity(), 1920, 1080, true);
        const Vector4f vertex(1, 2, -10, 1);
        const auto reference = uv(projection, vertex);
        const auto a = uv(standard.JitteredProjection(), vertex) - reference;
        const auto b = uv(retina.JitteredProjection(), vertex) - reference;
        CHECK_THAT(a.x * 960, WithinAbs(b.x * 1920, 1e-4));
        CHECK_THAT(a.y * 540, WithinAbs(b.y * 1080, 1e-4));
        CHECK(std::abs(b.x * 1920) <= 0.5f);
        CHECK(std::abs(b.y * 1080) <= 0.5f);
        standard.Commit();
        retina.Commit();
    }
    // Display scale changes while the logical window size and aspect remain unchanged.
    retina.Begin(projection, Matrix4f::Identity(), 960, 540, true);
    CHECK_FALSE(retina.valid);
    CHECK(retina.frame == 0);
    CHECK(retina.models.empty());
}

TEST_CASE (
"Temporal timings identify fresh submissions across history resets"
,
"[temporal]"
)
{
    TemporalTimings timings;
    CHECK(timings.Publish(12, 3, {1, 2, 3, 4}));
    CHECK_FALSE(timings.Publish(12, 1, {9, 9, 9, 9}));
    CHECK_FALSE(timings.Publish(10, 1, {9, 9, 9, 9}));
    CHECK(timings.mode == 3);
    CHECK(timings.milliseconds[2] == 3);
    // A new pass/query pool must keep using the renderer's monotonic submission identity.
    CHECK(timings.Publish(20, 2, {4, 3, 0, 1}));
    CHECK(timings.frame == 20);
    CHECK(timings.mode == 2);
}