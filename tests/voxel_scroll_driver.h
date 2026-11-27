#pragma once
#include "tests/voxel_readback.h"
#include "tests/voxel_cascade_traversal.h"

class VoxelScrollDriver : public Prehistoric::Layer {
public:
    explicit VoxelScrollDriver(Prehistoric::Scene *scene) : scene(scene) {
    }

    void Render() override { ++frames; }

    void Update(float) override {
        using namespace Prehistoric;
        Input::getInstance().setPauseRendering(false);
        if (frame == frames) return;
        frame = frames;
        auto *core = Application::Get().getEngineLayer();
        auto *renderer = core->getRenderingEngine()->getRenderer();
        auto *camera = renderer->getCamera();
        using Mode = Renderer::VoxelUpdate;
        auto expect = [&](Mode::Mode mode) {
            if (renderer->voxelUpdate.mode != mode)
                throw std::runtime_error("Unexpected voxel update mode at frame " + std::to_string(frame));
        };
        auto setWidth = [&](float width) {
            const auto volume = renderer->GetVoxelVolume();
            renderer->SetVoxelVolume({volume.x, volume.y, volume.z}, width);
        };
        auto compare = [&]() {
            auto fresh = ReadVoxelCascades(*renderer);
            if (fresh != scrolled) {
                size_t geometry = 0, light = 0;
                for (size_t i = 0; i < fresh.size(); ++i)
                    if (fresh[i] != scrolled[i]) (i % (5 * 64 * 64 * 64) < 3 * 64 * 64 * 64 ? geometry : light)++;
                throw std::runtime_error(
                    "Scroll/full mismatch: geometry=" + std::to_string(geometry) + " light=" + std::to_string(light));
            }
        };
        if (frame == 1) {
            renderer->voxelCascadeCount = 4;
            renderer->voxelFollowCamera = true;
            renderer->voxeliseTerrain = false;
            renderer->terrainReceivesGI = false;
            renderer->temporal = {};
            renderer->screenSpaceReflections = false;
            renderer->ambientOcclusion.enabled = false;
            renderer->SetVoxelVolume({0, 0, 0}, 32);
            camera->setPosition({0, 0, 0});
            if (auto *vk = dynamic_cast<VKRenderer *>(renderer)) {
                vk->voxelGI = true;
                vk->voxelView = false;
                vk->voxelResolution = 64;
                vk->voxelBounces = 3;
                vk->voxelDirectional = true;
            }
            if (auto *gl = dynamic_cast<GLRenderer *>(renderer)) {
                auto &s = gl->getVoxelPass()->GetSettings();
                s.globalIllumination = true;
                s.debugView = false;
                s.resolutionIndex = 0;
                s.traceDepth = 3;
                s.directionalMips = true;
            }
            auto *manager = core->getAssetManager();
            auto *assets = manager->getAssetManager();
            auto mesh = assets->loadVertexBuffer(std::nullopt, "engine://models/cube.obj").value();
            auto shader = assets->loadShader(ShaderName::PBR).value();
            auto pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, mesh);
            auto material = manager->storeMaterial(new Material(assets));
            material->addVector3f(COLOUR, {0.7f, 0.2f, 0.1f});
            material->addVector4f(MROT, {0, 0.6f, 1, 0});
            material->addFloat(EMISSION, 3);
            const Vector3f positions[] = {
                {0, 0, -4}, {16.5f, 0, 0}, {-16.5f, 0, 0},
                {0, 16.5f, 0}, {0, -16.5f, 0}, {0, 0, 16.5f}, {0, 0, -16.5f},
                {24, 0, -4}, {48, 0, -4}, {96, 0, -4}
            };
            for (unsigned i = 0; i < std::size(positions); ++i) {
                auto *object = new GameObject();
                object->SetPosition(positions[i]);
                object->SetScale(i >= 7 ? Vector3f(8) : Vector3f(1.3f, 1.7f, 1.1f));
                object->AddComponent(RENDERER_COMPONENT,
                                     new RendererComponent(renderer->getWindow(), manager, pipeline, material));
                scene->AddGameObject("scroll cube " + std::to_string(i), object);
                if (i == 0) moving = object;
            }
        }
        if (frame == 3) {
            ValidateCascadeTraversal(*renderer);
            auto initial = ReadVoxelCascades(*renderer);
            if (std::count_if(initial.begin(), initial.begin() + 64 * 64 * 64, [](auto v) { return v != 0; }) < 50)
                throw std::runtime_error("Scroll fixture has insufficient occupied cells");
            for (unsigned c = 1; c < 4; ++c) {
                const auto begin = initial.begin() + c * 5 * 64 * 64 * 64;
                if (std::none_of(begin, begin + 64 * 64 * 64, [](auto v) { return v != 0; }))
                    throw std::runtime_error("Distant cascade has no geometry");
            }
            camera->setPosition({0.1f, 0, 0});
        }
        if (frame == 4) {
            expect(Mode::Reused);
            camera->setForward({1, 0, 0});
        }
        if (frame == 5) {
            expect(Mode::Reused);
            camera->setPosition({0.5f, 0, 0});
        }
        if (frame == 6) {
            expect(Mode::Scrolled);
            if (renderer->voxelCascadeUpdates[2].mode != Mode::Reused ||
                renderer->voxelCascadeUpdates[3].mode != Mode::Reused)
                throw std::runtime_error("Coarse grids did not retain their independent snapped centres");
            if (renderer->voxelUpdate.dirtyCells != 4096)
                throw std::runtime_error(
                    "Single-cell scroll dirtied retained cells");
            scrolled = ReadVoxelCascades(*renderer);
            renderer->voxelFollowCamera = false;
            setWidth(33);
        }
        if (frame == 7) {
            expect(Mode::Rebuilt);
            setWidth(32);
        }
        if (frame == 8) {
            expect(Mode::Rebuilt);
            compare();
            renderer->voxelFollowCamera = true;
            camera->setPosition({-0.5f, 0.5f, -1});
        }
        if (frame == 9) {
            expect(Mode::Scrolled);
            scrolled = ReadVoxelCascades(*renderer);
            renderer->voxelFollowCamera = false;
            setWidth(33);
        }
        if (frame == 10) {
            expect(Mode::Rebuilt);
            setWidth(32);
        }
        if (frame == 11) {
            expect(Mode::Rebuilt);
            compare();
            renderer->voxelFollowCamera = true;
            camera->setPosition({100, 0, 0});
        }
        if (frame == 12) {
            expect(Mode::Rebuilt);
            camera->setPosition({0, 0, 0});
        }
        if (frame == 13) {
            expect(Mode::Rebuilt);
            moving->Move({3, 0, 0});
        }
        if (frame == 14) {
            expect(Mode::Rebuilt);
            scene->RemoveGameObject(moving);
        }
        if (frame == 15) expect(Mode::Rebuilt);
        if (frame == 16) {
            expect(Mode::Reused);
            renderer->voxelCascadeCount = 1;
        }
        if (frame == 17) {
            if (ReadVoxelCascades(*renderer).size() != 5 * 64 * 64 * 64)
                throw std::runtime_error("Single cascade reconfiguration failed");
            renderer->voxelCascadeCount = 4;
        }
        if (frame == 19) {
            if (ReadVoxelCascades(*renderer).size() != 20 * 64 * 64 * 64)
                throw std::runtime_error("Four cascade reconfiguration failed");
            renderer->voxelDebugCascade = 3;
            renderer->voxelCascadeColours = true;
            if (auto *vk = dynamic_cast<VKRenderer *>(renderer)) vk->voxelView = true;
            if (auto *gl = dynamic_cast<GLRenderer *>(renderer)) gl->getVoxelPass()->GetSettings().debugView = true;
        }
        if (frame == 20) renderer->voxelDebugCascade = 0;
        if (frame == 21) {
            if (auto *vk = dynamic_cast<VKRenderer *>(renderer)) vk->voxelView = false;
            if (auto *gl = dynamic_cast<GLRenderer *>(renderer)) gl->getVoxelPass()->GetSettings().debugView = false;
        }
        if (frame == 23) {
            std::fprintf(
                stderr,
                "VOXEL SCROLL PASS: subcell/rotation reuse, four cascades axis/diagonal scroll equals full geometry and radiance, resize/teleport/mutation fallback\n");
            Application::Get().Stop();
        }
    }

private:
    Prehistoric::Scene *scene;
    Prehistoric::GameObject *moving = nullptr;
    unsigned frame = 0, frames = 0;
    std::vector<uint32_t> scrolled;
};