#pragma once
#include "tests/temporal_render_driver.h"
#include "prehistoric/common/rendering/TerrainVoxel.h"
#include "platform/vulkan/rendering/command/VKCommandPool.h"
#include <fstream>
#include <json.hpp>

class TerrainVoxelDriver : public Prehistoric::Layer {
public:
    explicit TerrainVoxelDriver(Prehistoric::Scene *scene) : scene(scene) {
    }

    void Render() override { ++frames; }

    void Update(float) override {
        using namespace Prehistoric;
        Input::getInstance().setPauseRendering(false);
        if (frames == frame) return;
        frame = frames;
        auto *core = Application::Get().getEngineLayer();
        auto *renderer = core->getRenderingEngine()->getRenderer();
        auto *camera = renderer->getCamera();
        camera->setPosition({0, 12, 20});
        camera->setForward(Vector3f(0, 0.4f, 1).normalise());
        camera->setViewMatrix(Matrix4f::View(camera->getForward(), camera->getUp()) *
                              Matrix4f::Translation(camera->getPosition() * -1));
        camera->setViewProjectionMatrix(camera->getProjectionMatrix() * camera->getViewMatrix());
        if (frame == 1) {
            renderer->temporal = {};
            renderer->voxelFollowCamera = false;
            renderer->terrainReceivesGI = scene->terrainReceivesGI = true;
            renderer->screenSpaceReflections = false;
            if (auto *vk = dynamic_cast<VKRenderer *>(renderer)) {
                vk->voxelGI = true;
                vk->voxelResolution = 64;
                vk->voxelExtent = 32;
                vk->voxelCenter = Vector3f(0);
                vk->voxelBounces = 1;
            }
            if (auto *gl = dynamic_cast<GLRenderer *>(renderer)) {
                auto &s = gl->getVoxelPass()->GetSettings();
                s.globalIllumination = true;
                s.resolutionIndex = 0;
                s.extent = 32;
                s.center = Vector3f(0);
                s.traceDepth = 1;
            }
            TerrainConfig cfg = __TerrainConfig;
            cfg.scaleXZ = 24;
            cfg.scaleY = 8;
            cfg.tessellationFactor = 4;
            cfg.lodRanges = {0};
            cfg.lodMorphingAreas = {0};
            terrain = new Terrain(renderer->getWindow(), renderer->getCamera(), core->getAssetManager(),
                                  std::getenv("PREHISTORIC_TEST_TERRAIN_CONFIG"), cfg);
            terrain->SetPosition({-12, 0, -12});
            terrain->SetScale({24, 8, 24});
            scene->AddGameObject("Voxel terrain", terrain);
            renderer->voxeliseTerrain = scene->voxeliseTerrain = false;
        }
        if (frame == 4) {
            baseline = Read(*renderer);
            renderer->voxeliseTerrain = scene->voxeliseTerrain = true;
        }
        if (frame == 8) {
            baked = Read(*renderer);
            size_t added = 0;
            for (size_t i = 0; i < baked.size(); ++i) added += baked[i] != 0 && baseline[i] == 0;
            if (added < 1000) throw std::runtime_error("Terrain did not contribute expected voxel coverage");
            // The fixture is a flat plane at 128/255 * 8. Its interior must have no holes.
            for (int z = 10; z < 54; ++z)
                for (int x = 10; x < 54; ++x)
                    if (!baked[x + 64 * (40 + 64 * z)]) throw std::runtime_error("Hole in flat terrain coverage");
            std::fprintf(stderr, "TERRAIN added cells: %zu\n", added);
            WorldSerialiser(renderer->getWindow(), core->getAssetManager()).SerialiseWorldJSON(
                "terrain-roundtrip.json", scene);
            std::ifstream file("terrain-roundtrip.json");
            nlohmann::json saved;
            file >> saved;
            if (!saved.value("voxelise_terrain", false) || !saved.value("terrain_receives_gi", false))
                throw
                        std::runtime_error("Terrain voxel setting was not saved");
            terrain->Move({40, 0, 0});
        }
        if (frame == 12) {
            if (Read(*renderer) != baseline) throw std::runtime_error("Moved terrain left stale cells");
            terrain->Move({-40, 0, 0});
        }
        if (frame == 16) {
            if (Read(*renderer) != baked) throw std::runtime_error("Restored terrain coverage changed");
            renderer->voxeliseTerrain = scene->voxeliseTerrain = false;
        }
        if (frame == 20) {
            if (Read(*renderer) != baseline) throw std::runtime_error("Disabling terrain left stale cells");
            if (std::getenv("PREHISTORIC_TEMPORAL_WATER")) {
                WaterConfig cfg = __WaterConfig;
                cfg.scaleXZ = 24;
                cfg.scaleY = 0;
                cfg.displacementScale = 0.5f;
                cfg.tessellationFactor = 4;
                cfg.lodRanges = {0};
                cfg.lodMorphingAreas = {0};
                water = new Water(renderer->getWindow(), renderer->getCamera(), core->getAssetManager(), {0, 6, 0},
                                  cfg);
                scene->AddGameObject("Receiving water", water);
                renderer->voxeliseTerrain = scene->voxeliseTerrain = true;
            } else Finish();
        }
        if (water && (frame == 24 || frame == 28)) {
            if (Read(*renderer) != baked) throw std::runtime_error("Water animation changed voxel coverage");
            water->SetSimulationTime(float(frame));
            if (frame == 28) {
                if (auto *vk = dynamic_cast<VKRenderer *>(renderer)) vk->voxelGI = false;
                if (auto *gl = dynamic_cast<GLRenderer *>(renderer))
                    gl->getVoxelPass()->GetSettings().globalIllumination = false;
            }
        }
        if (water && frame == 32) Finish();
    }

private:
    static void Finish() {
        std::fprintf(
            stderr, "TERRAIN PASS: rendered frames, coverage, movement, toggle, persistence and receiver lifecycle\n");
        Prehistoric::Application::Get().Stop();
    }

    static std::vector<uint32_t> Read(Prehistoric::Renderer &renderer) {
        using namespace Prehistoric;
        std::vector<uint32_t> words(64 * 64 * 64);
        if (auto *gl = dynamic_cast<GLRenderer *>(&renderer)) {
            glFinish();
            glGetTextureImage(gl->getVoxelPass()->GetOccluderTexture(), 0, GL_RED_INTEGER, GL_UNSIGNED_INT,
                              GLsizei(words.size() * 4), words.data());
            return words;
        }
        auto *vk = static_cast<VKRenderer *>(&renderer);
        auto *gpu = static_cast<VKDevice *>(renderer.getWindow()->getContext()->getDevices());
        vkDeviceWaitIdle(gpu->getDevice());
        VKBuffer output(gpu, words.size() * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VKCommandPool commands(gpu);
        commands.AddCommandBuffer();
        auto *record = commands.getCommandBuffer(0);
        record->BindBuffer();
        auto cmd = record->getCommandBuffer();
        VkMemoryBarrier barrier{
            VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT
        };
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &barrier, 0,
                             nullptr, 0, nullptr);
        VkBufferCopy copy{0, 0, words.size() * 4};
        vkCmdCopyBuffer(cmd, vk->GetVoxelGeometryBuffer(), output.getBuffer(), 1, &copy);
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &barrier, 0,
                             nullptr, 0, nullptr);
        record->UnbindBuffer();
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;
        auto queue = gpu->getGraphicsQueue().getQueue();
        if (vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE) != VK_SUCCESS || vkQueueWaitIdle(queue) != VK_SUCCESS)
            throw std::runtime_error("Terrain voxel readback submission failed");
        void *mapped = nullptr;
        if (vkMapMemory(gpu->getDevice(), output.getMemory(), 0, output.getSize(), 0, &mapped) != VK_SUCCESS)
            throw std::runtime_error("Terrain voxel readback mapping failed");
        std::memcpy(words.data(), mapped, output.getSize());
        vkUnmapMemory(gpu->getDevice(), output.getMemory());
        return words;
    }

    Prehistoric::Scene *scene;
    Prehistoric::Terrain *terrain = nullptr;
    Prehistoric::Water *water = nullptr;
    unsigned frame = 0, frames = 0;
    std::vector<uint32_t> baseline, baked;
};