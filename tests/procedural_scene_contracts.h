#pragma once
#include "prehistoric/core/modules/water/Water.h"
#include "prehistoric/core/modules/atmosphere/OrbitingSun.h"
#include "prehistoric/core/modules/gui/button/GUIButton.h"
#include "prehistoric/core/modules/gui/slider/GUISlider.h"

// Run with a live device so constructors, destruction and generated-resource ownership are exercised.
inline void ProceduralSceneContracts(Prehistoric::Scene &scene, Prehistoric::Renderer &renderer) {
    using namespace Prehistoric;
    auto require = [](bool condition, const char *message) {
        if (!condition) throw std::runtime_error(message);
    };
    auto *manager = Application::Get().getEngineLayer()->getAssetManager();
    auto *window = renderer.getWindow();
    auto *camera = renderer.getCamera();
    auto group = std::make_unique<GameObject>();
    auto terrainSettings = __TerrainConfig;
    terrainSettings.scaleXZ = 128;
    terrainSettings.scaleY = 12;
    terrainSettings.lodRanges = {100, 30};
    terrainSettings.lodMorphingAreas = {84, 22};
    auto *terrain = new Terrain(window, camera, manager, std::getenv("PREHISTORIC_TEST_TERRAIN_CONFIG"),
                                terrainSettings);
    terrain->SetPosition({-64, -15, -64});
    terrain->AddChild("Terrain attachment", new GameObject());
    group->AddChild("Procedural terrain", terrain);
    const auto defaultResolution = __WaterConfig.fftResolution;
    auto waterSettings = __WaterConfig;
    waterSettings.fftResolution = 64;
    waterSettings.scaleXZ = 80;
    waterSettings.lodRanges = {80, 20};
    waterSettings.lodMorphingAreas = {70, 15};
    auto *water = new Water(window, camera, manager, {0, -2, 0}, waterSettings);
    water->SetSimulationTime(3.5f);
    group->AddChild("Procedural water", water);
    auto *sun = new OrbitingSun();
    sun->SetPhase(27);
    group->AddChild("Procedural sun", sun);
    bool buttonData = true;
    float sliderData = 0.25f;
    auto *button = new GUIButton(window, manager, {1, 0, 0}, &buttonData, sizeof(buttonData), false);
    auto *slider = new GUISlider(window, manager, 0, 1, {0, 1, 0}, &sliderData, sizeof(sliderData), false);
    slider->setProgress(0.25f);
    group->AddChild("Procedural button", button);
    group->AddChild("Procedural slider", slider);
    auto *preview = new GUIElement(window, manager, Vector3f(1), nullptr, 0, false);
    preview->setTexture(terrain->getMaps()->getNormalmap());
    group->AddChild("Procedural preview", preview);
    auto *unsized = new GUIElement(window, manager, Vector3f(1), &buttonData, 0, false);
    scene.AddGameObject("Uncopyable data fixture", unsized);
    const auto before = scene.getSceneRoot()->getAllChildren().size();
    bool rejected = false;
    try { scene.Duplicate(*unsized); } catch (const std::exception &) { rejected = true; }
    require(rejected && scene.getSceneRoot()->getAllChildren().size() == before, "Failed duplication was not atomic");
    scene.RemoveGameObject(unsized);
    auto *source = group.get();
    scene.AddGameObject("Procedural group", group.release());
    auto *copy = scene.Duplicate(*source);
    auto *terrainCopy = dynamic_cast<Terrain *>(copy->getChild("Procedural terrain copy"));
    auto *waterCopy = dynamic_cast<Water *>(copy->getChild("Procedural water copy"));
    auto *sunCopy = dynamic_cast<OrbitingSun *>(copy->getChild("Procedural sun copy"));
    auto *buttonCopy = dynamic_cast<GUIButton *>(copy->getChild("Procedural button copy"));
    auto *sliderCopy = dynamic_cast<GUISlider *>(copy->getChild("Procedural slider copy"));
    auto *previewCopy = dynamic_cast<GUIElement *>(copy->getChild("Procedural preview copy"));
    require(terrainCopy && waterCopy && sunCopy && buttonCopy && sliderCopy && previewCopy,
            "Specialised duplication lost concrete types or names");
    require(terrainCopy->getQuadtree() != terrain->getQuadtree() &&
            terrainCopy->getMaps()->getNormalmap() != terrain->getMaps()->getNormalmap(),
            "Terrain aliases generated resources");
    require(terrainCopy->getChild("Terrain attachment copy") != nullptr, "Terrain lost authored child");
    require(previewCopy->getTexture() == terrainCopy->getMaps()->getNormalmap(),
            "GUI reference was not remapped to copied terrain");
    require(buttonCopy->getData() != button->getData() && *static_cast<bool *>(buttonCopy->getData()),
            "Button did not own copied data");
    *static_cast<bool *>(buttonCopy->getData()) = false;
    *static_cast<float *>(sliderCopy->getData()) = 0.75f;
    require(buttonData && sliderData == 0.25f, "GUI copy changed source payload");
    require(std::abs(sunCopy->GetPhase() - sun->GetPhase()) < 0.01, "Orbiting sun lost phase");
    require(std::abs(waterCopy->GetSimulationTime() - water->GetSimulationTime()) < 0.05,
            "Water lost simulation phase");
    require(waterCopy->GetSurfaceMaterial() != water->GetSurfaceMaterial(), "Water shares mutable material");
    if (__FrameworkConfig.api == Vulkan) require(&waterCopy->simulation() != &water->simulation(),
                                                 "Water shares simulation");
    else require(waterCopy->getReflectionTexture() != water->getReflectionTexture(), "Water shares capture targets");
    terrainCopy->SetPosition({70, -10, -64});
    terrainCopy->SetRotation({0, 20, 0});
    waterCopy->SetPosition({60, 1, -40});
    waterCopy->GetSettings().baseColour = {0.1f, 0.5f, 0.2f};
    auto layers = terrain->getMaps()->getMaterials();
    auto copiedLayers = terrainCopy->getMaps()->getMaterials();
    const float originalTiling = layers.front()->getFloat("horizontalScale");
    copiedLayers.front()->getFloats()["horizontalScale"] = originalTiling + 2;
    require(layers.front()->getFloat("horizontalScale") == originalTiling, "Terrain layers share mutable state");
    require(water->GetSettings().baseColour == waterSettings.baseColour, "Water settings are shared");
    require(__WaterConfig.fftResolution == defaultResolution, "Procedural settings changed defaults");
    const auto copyName = copy->getName();
    WorldSerialiser(window, manager).SerialiseWorldJSON("procedural-roundtrip.json", &scene);
    auto loaded = std::make_unique<Scene>("procedural-roundtrip.json", window, camera, manager);
    auto *restored = dynamic_cast<GameObject *>(loaded->getSceneRoot()->getChild(copyName));
    require(restored != nullptr, "Procedural group did not survive save/load");
    auto *restoredTerrain = dynamic_cast<Terrain *>(restored->getChild("Procedural terrain copy"));
    auto *restoredWater = dynamic_cast<Water *>(restored->getChild("Procedural water copy"));
    auto *restoredPreview = dynamic_cast<GUIElement *>(restored->getChild("Procedural preview copy"));
    require(restoredTerrain && restoredWater && restoredPreview, "Save/load flattened specialised objects");
    require(restoredTerrain->getWorldTransform().getPosition().x == 70 && restoredWater->height() == 1,
            "Save/load lost procedural placement");
    require(restoredTerrain->GetSettings().scaleXZ == 128 && restoredWater->GetSettings().fftResolution == 64,
            "Save/load lost procedural settings");
    require(restoredTerrain->getMaps()->getMaterials().front()->getFloat("horizontalScale") == originalTiling + 2,
            "Save/load lost procedural layer edits");
    require(restoredPreview->getTexture() == restoredTerrain->getMaps()->getNormalmap(),
            "Save/load lost generated texture reference");
    require(dynamic_cast<OrbitingSun *>(restored->getChild("Procedural sun copy")) != nullptr,
            "Save/load lost sun behaviour");
    // Exercise both destruction orders before rendering the surviving copy for several frames.
    loaded.reset();
    for (unsigned i = 0; i < 3; ++i) {
        auto *temporary = scene.Duplicate(*source);
        scene.RemoveGameObject(temporary);
    }
    button->OwnData(&buttonData, sizeof(buttonData));
    slider->OwnData(&sliderData, sizeof(sliderData));
    require(previewCopy->getTexture() == terrainCopy->getMaps()->getNormalmap(),
            "Copied preview lost generated texture");
    loaded = std::make_unique<Scene>("procedural-roundtrip.json", window, camera, manager);
    loaded->ValidateSky();
    camera->setPosition({100, 100, -150});
    camera->setForward((Vector3f(70, 0, 0) - camera->getPosition()).normalise());
    camera->setUp({0, 1, 0});
    camera->setViewMatrix(
        Matrix4f::View(camera->getForward(), camera->getUp()) * Matrix4f::Translation(camera->getPosition() * -1));
    camera->setViewProjectionMatrix(camera->getProjectionMatrix() * camera->getViewMatrix());
    std::fprintf(
        stderr,
        "PROCEDURAL CONTRACT PASS: types, owned resources, payloads, reference remapping, placement, save/load and deletion orders\n");
}