#pragma once
// Native scene contracts and first-render checks, selected by --world-suite.
#include "tests/temporal_render_driver.h"
#include "prehistoric/core/modules/atmosphere/AtmosphereCapture.h"
#include "prehistoric/core/node/component/VolumetricSphereComponent.h"
#include <filesystem>
#include <fstream>
#include <json.hpp>
#include "tests/procedural_scene_contracts.h"

class WorldSceneDriver : public Prehistoric::Layer {
public:
    explicit WorldSceneDriver(Prehistoric::Scene *scene) : original(scene) {
        std::ifstream manifest(std::getenv("PREHISTORIC_WORLD_MANIFEST"));
        std::string path;
        while (std::getline(manifest, path)) if (!path.empty()) worlds.push_back(path);
        if (worlds.empty()) throw std::runtime_error("World manifest is empty");
    }

    void Render() override { ++renderedFrames; }

    void Update(float) override {
        Prehistoric::Input::getInstance().setPauseRendering(false);
        if (renderedFrames == frame) return;
        frame = renderedFrames;
        using namespace Prehistoric;
        auto &app = Application::Get();
        auto *core = app.getEngineLayer();
        auto *renderer = core->getRenderingEngine()->getRenderer();
        if (!renderer->CanRenderFrame()) return;
        if (auto *vk = dynamic_cast<VKRenderer *>(renderer)) vk->voxelGI = false;
        if (auto *gl = dynamic_cast<GLRenderer *>(renderer))
            gl->getVoxelPass()->GetSettings().globalIllumination = false;
        if (__FrameworkConfig.api == Vulkan)
            vkDeviceWaitIdle(static_cast<VKDevice *>(renderer->getWindow()->getContext()->getDevices())->getDevice());
        else glFinish();
        if (frame == 1) {
            Contracts(*original, *renderer);
            ProceduralSceneContracts(*original, *renderer);
        }
        if (frame == 3) {
            auto *source = dynamic_cast<GameObject *>(original->getSceneRoot()->getChild("Procedural group"));
            auto *copy = dynamic_cast<GameObject *>(original->getSceneRoot()->getChild("Procedural group copy"));
            auto *terrain = dynamic_cast<Terrain *>(copy->getChild("Procedural terrain copy"));
            auto *water = dynamic_cast<Water *>(copy->getChild("Procedural water copy"));
            if (water->getLeaves().empty() || terrain->getQuadtree()->getLeaves().empty())
                throw std::runtime_error("Duplicated procedural geometry has no rendered patches");
            auto *patch = dynamic_cast<TerrainNode *>(terrain->getQuadtree()->getChildren().begin()->second);
            const auto expected = terrain->getWorldTransform().getTransformationMatrix() *
                                  Vector4f(patch->getLocation().x + patch->getGap() / 2, 0,
                                           patch->getLocation().y + patch->getGap() / 2, 1);
            if (std::abs(patch->getWorldPosition().x - expected.x) > 0.01 ||
                std::abs(patch->getWorldPosition().z - expected.z) > 0.01)
                throw std::runtime_error("Terrain patch positions did not follow copied placement");
            original->RemoveGameObject(source);
            std::fprintf(
                stderr,
                "PROCEDURAL RENDER PASS: multiple terrain/water instances, transformed patches and source deletion\n");
        }
        if (frame % 4 != 0) return;
        if (current) std::fprintf(stderr, "WORLD RENDER PASS: %s\n", worlds[index - 1].c_str());
        if (index == worlds.size()) {
            core->SetScene(original);
            current.reset();
            std::fprintf(stderr, "WORLD SUITE PASS: %zu worlds loaded, round-tripped and rendered\n", worlds.size());
            app.Stop();
            return;
        }
        const auto file = std::filesystem::path(worlds[index++]);
        std::fprintf(stderr, "WORLD LOAD: %s\n", file.string().c_str());
        const auto previous = std::filesystem::current_path();
        auto appDirectory = file.parent_path();
        while (appDirectory.filename() != "res" && appDirectory.has_parent_path())
            appDirectory = appDirectory.parent_path();
        std::filesystem::current_path(appDirectory.parent_path());
        auto next = std::make_unique<Scene>(file.string(), renderer->getWindow(), renderer->getCamera(),
                                            core->getAssetManager());
        next->ValidateSky();
        std::fprintf(stderr, "WORLD SERIALISE: %s\n", file.string().c_str());
        const auto roundtrip = previous / "world-roundtrip.json";
        WorldSerialiser(renderer->getWindow(), core->getAssetManager()).SerialiseWorldJSON(
            roundtrip.string(), next.get());
        std::fprintf(stderr, "WORLD RELOAD: %s\n", file.string().c_str());
        auto loaded = std::make_unique<Scene>(roundtrip.string(), renderer->getWindow(), renderer->getCamera(),
                                              core->getAssetManager());
        loaded->ValidateSky();
        if (bool(dynamic_cast<EnvironmentMap *>(next->GetSky())) != bool(
                dynamic_cast<EnvironmentMap *>(loaded->GetSky())))
            throw std::runtime_error("Round-trip changed sky type");
        std::filesystem::current_path(previous);
        core->SetScene(next.get());
        current = std::move(next);
    }

private:
    static void Contracts(Prehistoric::Scene &scene, Prehistoric::Renderer &renderer) {
        using namespace Prehistoric;
        auto rejected = [](auto operation) {
            try { operation(); } catch (const std::exception &) { return; }
            throw std::runtime_error("Invalid sky mutation was accepted");
        };
        rejected([&] { scene.RemoveGameObject(scene.GetSky()); });
        rejected([&] { scene.GetSky()->setEnabled(false); });
        rejected([&] { scene.Duplicate(*scene.GetSky()); });
        auto second = std::make_unique<EnvironmentMap>(__EnvironmentMapConfig.environmentMapLocation);
        rejected([&] { scene.AddGameObject("Second sky", second.get()); });
        if (auto *sky = dynamic_cast<Atmosphere *>(scene.GetSky())) {
            rejected([&] { scene.RemoveGameObject(sky->getSun()->getParent()); });
            rejected([&] { sky->getSun()->getParent()->RemoveComponent<Light>(); });
        }
        auto environment = std::make_unique<EnvironmentMap>(__EnvironmentMapConfig.environmentMapLocation);
        auto *map = environment.get();
        const auto asset = map->GetAsset();
        rejected([&] { map->SetAsset("/missing/environment.hdr"); });
        if (map->GetAsset() != asset) throw std::runtime_error("Failed HDR load changed source");
        scene.ReplaceSky(std::move(environment));
        AtmosphereCapture cache;
        cache.Complete(EnvironmentCaptureSource::FromScene(nullptr, 0, map));
        map->SetPosition({5, 6, 7});
        map->SetScale({2, 3, 4});
        if (cache.NeedsBake(EnvironmentCaptureSource::FromScene(nullptr, 0, map), 0))
            throw std::runtime_error("Position/scale affected environment lighting");
        map->SetRotation({0, 90, 0});
        if (!cache.NeedsBake(EnvironmentCaptureSource::FromScene(nullptr, 0, map), 0))
            throw std::runtime_error("Rotation failed to invalidate environment lighting");
        auto group = std::make_unique<GameObject>();
        auto *volume = new VolumetricSphereComponent();
        volume->setDensity(0.2f);
        group->AddComponent("Volume", volume);
        group->AddChild("Child", new GameObject());
        auto *object = group.get();
        scene.AddGameObject("Duplicate fixture", group.release());
        auto *copy = scene.Duplicate(*object);
        copy->GetComponent<VolumetricSphereComponent>()->setDensity(0.7f);
        if (object->GetComponent<VolumetricSphereComponent>()->getDensity() != 0.2f ||
            copy->getAllChildren().size() != 1 || copy->getAllChildren().front() == object->getChild("Child"))
            throw std::runtime_error("Duplicate aliases component or child state");
        const std::string duplicateName = copy->getName();
        auto &app = Application::Get();
        WorldSerialiser(renderer.getWindow(), app.getEngineLayer()->getAssetManager()).SerialiseWorldJSON(
            "duplicate-roundtrip.json", &scene);
        auto restored = std::make_unique<Scene>("duplicate-roundtrip.json", renderer.getWindow(), renderer.getCamera(),
                                                app.getEngineLayer()->getAssetManager());
        auto *restoredCopy = dynamic_cast<GameObject *>(restored->getSceneRoot()->getChild(duplicateName));
        if (!restoredCopy || restoredCopy->getAllChildren().size() != 1)
            throw std::runtime_error("Duplicate hierarchy failed save/load");
        auto *restoredSky = dynamic_cast<EnvironmentMap *>(restored->GetSky());
        if (!restoredSky || restoredSky->getWorldTransform().getRotation().y != 90)
            throw std::runtime_error("Environment rotation failed save/load");
        restored->ClearObjects();
        restored->ValidateSky();
        if (restored->getSceneRoot()->getAllChildren().size() != 1)
            throw std::runtime_error("New world did not retain exactly one default sky");
        auto newSun = Atmosphere::CreateSun();
        auto newAtmosphere = std::make_unique<Atmosphere>();
        newAtmosphere->setSun(newSun->GetComponent<Light>());
        restored->AddGameObject("Replacement Sun", newSun.release());
        restored->ReplaceSky(std::move(newAtmosphere));
        restored->ValidateSky();
        restored->ClearObjects();
        restored->ValidateSky();
        nlohmann::json saved;
        std::ifstream("duplicate-roundtrip.json") >> saved;
        auto write = [&](const nlohmann::json &data) {
            std::ofstream("migration-fixture.json") << data;
        };
        auto conflict = saved;
        conflict["environment_map"] = asset;
        write(conflict);
        rejected([&] {
            Scene invalid("migration-fixture.json", renderer.getWindow(), renderer.getCamera(),
                          app.getEngineLayer()->getAssetManager());
        });
        auto legacy = saved;
        legacy.erase("sky_schema");
        legacy["world_objects"] = nlohmann::json::array();
        legacy["environment_map"] = asset;
        write(legacy);
        Scene migrated("migration-fixture.json", renderer.getWindow(), renderer.getCamera(),
                       app.getEngineLayer()->getAssetManager());
        migrated.ValidateSky();
        if (!dynamic_cast<EnvironmentMap *>(migrated.GetSky())) throw std::runtime_error("Legacy HDR was not migrated");
        legacy.erase("environment_map");
        write(legacy);
        Scene fallback("migration-fixture.json", renderer.getWindow(), renderer.getCamera(),
                       app.getEngineLayer()->getAssetManager());
        fallback.ValidateSky();
        for (auto *sourceObject: scene.getSceneRoot()->getAllChildren()) {
            auto *sourceRenderer = sourceObject->GetComponent<RendererComponent>();
            if (!sourceRenderer || !sourceRenderer->getMeshBuffer()) continue;
            sourceRenderer->EditSubmesh(0)->getFloats()["emission"] = 0.25f;
            auto *duplicate = scene.Duplicate(*sourceObject);
            auto *duplicateRenderer = duplicate->GetComponent<RendererComponent>();
            if (sourceRenderer->getTemporalIdentity() == duplicateRenderer->getTemporalIdentity() ||
                sourceRenderer->getSubmeshOverride(0) == duplicateRenderer->getSubmeshOverride(0))
                throw std::runtime_error("Renderer duplication aliases temporal identity or material override");
            duplicateRenderer->getSubmeshOverride(0)->getFloats()["emission"] = 0.75f;
            if (sourceRenderer->getSubmeshOverride(0)->getFloat("emission") != 0.25f)
                throw std::runtime_error("Duplicate material override changed the original");
            scene.RemoveGameObject(duplicate);
            sourceRenderer->ResetSubmesh(0);
            break;
        }
        scene.RemoveGameObject(copy);
        scene.RemoveGameObject(object);
        scene.ValidateSky();
        renderer.RequestEnvironmentBake();
        std::fprintf(stderr, "WORLD CONTRACT PASS: exclusivity, failed HDR replacement, rotation and duplication\n");
    }

    Prehistoric::Scene *original;
    std::unique_ptr<Prehistoric::Scene> current;
    std::vector<std::string> worlds;
    size_t index = 0;
    unsigned frame = 0, renderedFrames = 0;
};