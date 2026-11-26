#include "Includes.hpp"
#include "ProceduralWorld.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/core/modules/water/Water.h"
#include "prehistoric/core/modules/atmosphere/OrbitingSun.h"
#include "prehistoric/core/modules/gui/button/GUIButton.h"
#include "prehistoric/core/modules/gui/slider/GUISlider.h"

namespace Prehistoric {
    using Json = nlohmann::json;

    static Json WriteConfig(const TerrainConfig &value) {
        Json result;
        result["scaleY"] = value.scaleY;
        result["scaleXZ"] = value.scaleXZ;
        result["tessellationFactor"] = value.tessellationFactor;
        result["tessellationSlope"] = value.tessellationSlope;
        result["tessellationShift"] = value.tessellationShift;
        result["lodRanges"] = value.lodRanges;
        result["lodMorphingAreas"] = value.lodMorphingAreas;
        return result;
    }

    static void ReadConfig(const Json &record, TerrainConfig &value) {
        value.scaleY = record.value("scaleY", value.scaleY);
        value.scaleXZ = record.value("scaleXZ", value.scaleXZ);
        value.tessellationFactor = record.value("tessellationFactor", value.tessellationFactor);
        value.tessellationSlope = record.value("tessellationSlope", value.tessellationSlope);
        value.tessellationShift = record.value("tessellationShift", value.tessellationShift);
        value.lodRanges = record.value("lodRanges", value.lodRanges);
        value.lodMorphingAreas = record.value("lodMorphingAreas", value.lodMorphingAreas);
    }

    static Json WriteConfig(const WaterConfig &value) {
        Json result;
        result["scaleXZ"] = value.scaleXZ;
        result["scaleY"] = value.scaleY;
        result["tessellationFactor"] = value.tessellationFactor;
        result["tessellationSlope"] = value.tessellationSlope;
        result["tessellationShift"] = value.tessellationShift;
        result["lodRanges"] = value.lodRanges;
        result["lodMorphingAreas"] = value.lodMorphingAreas;
        result["baseColour"] = {value.baseColour.x, value.baseColour.y, value.baseColour.z};
        result["displacementScale"] = value.displacementScale;
        result["displacementRange"] = value.displacementRange;
        result["tiling"] = value.tiling;
        result["choppiness"] = value.choppiness;
        result["wavemotion"] = value.wavemotion;
        result["normalStrength"] = value.normalStrength;
        result["highDetailRange"] = value.highDetailRange;
        result["capillarStrength"] = value.capillarStrength;
        result["capillarDownsampling"] = value.capillarDownsampling;
        result["fresnelFactor"] = value.fresnelFactor;
        result["kReflection"] = value.kReflection;
        result["kRefraction"] = value.kRefraction;
        result["reflectionBlendfactor"] = value.reflectionBlendfactor;
        result["dudvDownsampling"] = value.dudvDownsampling;
        result["underwaterPostProcessing"] = value.underwaterPostProcessing;
        result["underwaterBlurfactor"] = value.underwaterBlurfactor;
        result["distortionDelta"] = value.distortionDelta;
        result["specularFactor"] = value.specularFactor;
        result["specularAmplifier"] = value.specularAmplifier;
        result["emissionFactor"] = value.emissionFactor;
        result["diffuse"] = value.diffuse;
        result["windDirection"] = {value.windDirection.x, value.windDirection.y};
        result["windSpeed"] = value.windSpeed;
        result["alignment"] = value.alignment;
        result["fftResolution"] = value.fftResolution;
        result["fftAmplitude"] = value.fftAmplitude;
        result["fftL"] = value.fftL;
        result["fftCapillarWavesSupression"] = value.fftCapillarWavesSupression;
        result["choppy"] = value.choppy;
        result["tDelta"] = value.tDelta;
        return result;
    }

    static void ReadConfig(const Json &record, WaterConfig &value) {
        value.scaleXZ = record.value("scaleXZ", value.scaleXZ);
        value.scaleY = record.value("scaleY", value.scaleY);
        value.tessellationFactor = record.value("tessellationFactor", value.tessellationFactor);
        value.tessellationSlope = record.value("tessellationSlope", value.tessellationSlope);
        value.tessellationShift = record.value("tessellationShift", value.tessellationShift);
        value.lodRanges = record.value("lodRanges", value.lodRanges);
        value.lodMorphingAreas = record.value("lodMorphingAreas", value.lodMorphingAreas);
        if (record.contains("baseColour")) {
            auto v = record.at("baseColour").get<std::array<float, 3> >();
            value.baseColour = {v[0], v[1], v[2]};
        }
        value.displacementScale = record.value("displacementScale", value.displacementScale);
        value.displacementRange = record.value("displacementRange", value.displacementRange);
        value.tiling = record.value("tiling", value.tiling);
        value.choppiness = record.value("choppiness", value.choppiness);
        value.wavemotion = record.value("wavemotion", value.wavemotion);
        value.normalStrength = record.value("normalStrength", value.normalStrength);
        value.highDetailRange = record.value("highDetailRange", value.highDetailRange);
        value.capillarStrength = record.value("capillarStrength", value.capillarStrength);
        value.capillarDownsampling = record.value("capillarDownsampling", value.capillarDownsampling);
        value.fresnelFactor = record.value("fresnelFactor", value.fresnelFactor);
        value.kReflection = record.value("kReflection", value.kReflection);
        value.kRefraction = record.value("kRefraction", value.kRefraction);
        value.reflectionBlendfactor = record.value("reflectionBlendfactor", value.reflectionBlendfactor);
        value.dudvDownsampling = record.value("dudvDownsampling", value.dudvDownsampling);
        value.underwaterPostProcessing = record.value("underwaterPostProcessing", value.underwaterPostProcessing);
        value.underwaterBlurfactor = record.value("underwaterBlurfactor", value.underwaterBlurfactor);
        value.distortionDelta = record.value("distortionDelta", value.distortionDelta);
        value.specularFactor = record.value("specularFactor", value.specularFactor);
        value.specularAmplifier = record.value("specularAmplifier", value.specularAmplifier);
        value.emissionFactor = record.value("emissionFactor", value.emissionFactor);
        value.diffuse = record.value("diffuse", value.diffuse);
        if (record.contains("windDirection")) {
            auto v = record.at("windDirection").get<std::array<float, 2> >();
            value.windDirection = {v[0], v[1]};
        }
        value.windSpeed = record.value("windSpeed", value.windSpeed);
        value.alignment = record.value("alignment", value.alignment);
        value.fftResolution = record.value("fftResolution", value.fftResolution);
        value.fftAmplitude = record.value("fftAmplitude", value.fftAmplitude);
        value.fftL = record.value("fftL", value.fftL);
        value.fftCapillarWavesSupression = record.value("fftCapillarWavesSupression", value.fftCapillarWavesSupression);
        value.choppy = record.value("choppy", value.choppy);
        value.tDelta = record.value("tDelta", value.tDelta);
    }

    static Json WriteMaterial(Material &material, AssetManager *assets) {
        Json result;
        result["floats"] = material.getFloats();
        result["ints"] = material.getInts();
        for (const auto &[key, value]: material.getVector2fs()) result["vec2"][key] = {value.x, value.y};
        for (const auto &[key, value]: material.getVector3fs()) result["vec3"][key] = {value.x, value.y, value.z};
        for (const auto &[key, value]: material.getVector4fs()) result["vec4"][key] = {
                                                                    value.x, value.y, value.z, value.w
                                                                };
        // Only immutable named assets are portable. Generated maps stay instance-owned.
        for (const auto &[key, texture]: material.getTextures())
            for (const auto &[name, resource]: assets->getIDMap())
                if (resource.type == ResourceType::Texture && resource.handle == texture.handle) {
                    result["textures"][key] = name;
                    break;
                }
        return result;
    }

    static void ReadMaterial(const Json &value, Material &material, AssetManager *assets) {
        if (value.contains("floats")) material.getFloats() = value.at("floats").get<std::unordered_map<std::string,
                                          float> >();
        if (value.contains("ints")) material.getInts() = value.at("ints").get<std::unordered_map<std::string, int> >();
        if (value.contains("vec2"))
            for (const auto &item: value.at("vec2").items()) {
                auto v = item.value().get<std::array<float, 2> >();
                material.getVector2fs()[item.key()] = {v[0], v[1]};
            }
        if (value.contains("vec3"))
            for (const auto &item: value.at("vec3").items()) {
                auto v = item.value().get<std::array<float, 3> >();
                material.getVector3fs()[item.key()] = {v[0], v[1], v[2]};
            }
        if (value.contains("vec4"))
            for (const auto &item: value.at("vec4").items()) {
                auto v = item.value().get<std::array<float, 4> >();
                material.getVector4fs()[item.key()] = {v[0], v[1], v[2], v[3]};
            }
        if (value.contains("textures"))
            for (const auto &item: value.at("textures").items())
                material.addTexture(item.key(), assets->loadTexture(item.value().get<std::string>()).value());
    }

    static std::map<std::string, Material *> ProceduralMaterials(const GameObject &object) {
        std::map<std::string, Material *> result;
        size_t layerIndex = 0;
        if (const auto *terrain = dynamic_cast<const Terrain *>(&object))
            for (auto layer: terrain->getMaps()->getMaterials())
                result["layer" + std::to_string(layerIndex++)] = layer.pointer;
        if (const auto *water = dynamic_cast<const Water *>(&object)) result["surface"] = water->GetSurfaceMaterial();
        else if (dynamic_cast<const Terrain *>(&object))
            if (auto *renderer = object.GetComponent<RendererComponent>())
                result["surface"] = renderer->getMaterialHandle().pointer;
        return result;
    }

    void CopyProceduralMaterials(const GameObject &source, GameObject &copy) {
        const auto from = ProceduralMaterials(source), to = ProceduralMaterials(copy);
        if (from.size() != to.size()) throw std::runtime_error("Procedural material layout changed while copying");
        for (const auto &[name, material]: from) to.at(name)->CopyPropertiesFrom(*material);
    }

    static std::map<std::string, Texture *> Outputs(GameObject &object) {
        if (auto *water = dynamic_cast<Water *>(&object))
            return {{"reflection", water->getReflectionTexture()}, {"refraction", water->getRefractionTexture()}};
        if (auto *terrain = dynamic_cast<Terrain *>(&object))
            return {
                {"height", terrain->getMaps()->getHeightmap()}, {"normal", terrain->getMaps()->getNormalmap()},
                {"splat", terrain->getMaps()->getSplatmap()}
            };
        return {};
    }

    void RemapProceduralReferences(const GameObject &source, GameObject &copy) {
        std::unordered_map<Texture *, Texture *> remap;
        std::vector<std::pair<const GameObject *, GameObject *> > pairs;
        std::function < void(const GameObject &, GameObject &) > collect = [&](const GameObject &a, GameObject &b) {
            pairs.emplace_back(&a, &b);
            auto outputs = Outputs(b);
            for (const auto &[name, texture]: Outputs(const_cast<GameObject &>(a)))
                if (texture && outputs.at(name)) remap[texture] = outputs.at(name);
            for (const auto &[name, child]: a.getChildren())
                if (!child->IsGenerated()) collect(*dynamic_cast<GameObject *>(child),
                                                   *dynamic_cast<GameObject *>(b.getChild(name)));
        };
        collect(source, copy);
        for (const auto &[a, b]: pairs)
            if (auto *gui = dynamic_cast<GUIElement *>(b))
                if (auto found = remap.find(gui->getTexture()); found != remap.end()) gui->setTexture(found->second);
    }

    Json WriteProceduralObject(GameObject &object, AssembledAssetManager *manager) {
        Json record = Json::object();
        auto *assets = manager->getAssetManager();
        if (auto *terrain = dynamic_cast<Terrain *>(&object)) {
            record["kind"] = "terrain";
            record["config"] = terrain->GetConfigFile();
            record["settings"] = WriteConfig(terrain->GetSettings());
        } else if (auto *water = dynamic_cast<Water *>(&object)) {
            record["kind"] = "water";
            record["settings"] = WriteConfig(water->GetSettings());
            record["phase"] = water->GetSimulationTime();
        } else if (auto *sun = dynamic_cast<OrbitingSun *>(&object)) {
            record["kind"] = "orbiting_sun";
            record["phase"] = sun->GetPhase();
        } else if (auto *gui = dynamic_cast<GUIElement *>(&object)) {
            record["kind"] = "gui";
            if (dynamic_cast<GUIButton *>(gui)) record["kind"] = "gui_button";
            if (auto *slider = dynamic_cast<GUISlider *>(gui)) {
                record["kind"] = "gui_slider";
                record["range"] = {slider->getMinValue(), slider->getMaxValue()};
                record["progress"] = slider->getProgress();
            }
            const auto colour = gui->getColour();
            record["colour"] = {colour.x, colour.y, colour.z};
            record["visible"] = gui->isVisible();
            if (gui->getData()) {
                const auto bytes = gui->getType() == GUIType::Button
                                       ? sizeof(bool)
                                       : gui->getType() == GUIType::Slider
                                             ? sizeof(float)
                                             : gui->getDataSize();
                if (!bytes) throw std::runtime_error("Cannot save unsized GUI data");
                const auto *data = static_cast<const uint8_t *>(gui->getData());
                record["data"] = std::vector<uint8_t>(data, data + bytes);
            }
            if (auto *texture = gui->getTexture()) {
                Node *root = &object;
                while (root->getParent()) root = root->getParent();
                for (auto *node: static_cast<GameObject *>(root)->getAllChildren())
                    for (const auto &[output, value]: Outputs(*node))
                        if (texture == value) record["texture_source"] = {
                                                  {"object", node->getName()}, {"output", output}
                                              };
                if (!record.contains("texture_source")) {
                    const auto handle = assets->FindTexture(texture).value();
                    for (const auto &[name, resource]: assets->getIDMap())
                        if (resource.type == ResourceType::Texture && resource.handle == handle.handle)
                            record["texture_asset"] = name;
                    if (!record.contains("texture_asset")) throw std::runtime_error(
                        "GUI texture has no portable asset or procedural source");
                }
            }
        }
        for (const auto &[name, material]: ProceduralMaterials(object))
            record["procedural_materials"][name] = WriteMaterial(*material, assets);
        return record;
    }

    std::unique_ptr<GameObject>
    ReadProceduralObject(const Json &record, Window *window, AssembledAssetManager *manager) {
        const auto kind = record.value("kind", std::string{});
        auto *camera = Application::Get().getEngineLayer()->getRenderingEngine()->getCamera();
        std::unique_ptr<GameObject> result;
        if (kind == "terrain") {
            auto config = __TerrainConfig;
            ReadConfig(record.at("settings"), config);
            result = std::make_unique<Terrain>(window, camera, manager, record.at("config").get<std::string>(), config);
        } else if (kind == "water") {
            auto config = __WaterConfig;
            ReadConfig(record.at("settings"), config);
            auto water = std::make_unique<Water>(window, camera, manager, Vector3f(0), config);
            water->SetSimulationTime(record.value("phase", 0.0f));
            result = std::move(water);
        } else if (kind == "orbiting_sun") {
            auto sun = std::make_unique<OrbitingSun>();
            sun->RemoveComponent<Light>();
            sun->SetPhase(record.value("phase", 0.0));
            result = std::move(sun);
        } else if (kind == "gui" || kind == "gui_button" || kind == "gui_slider") {
            std::unique_ptr<GUIElement> gui;
            if (kind == "gui_button") gui = std::make_unique<GUIButton>(window, manager);
            else if (kind == "gui_slider") {
                auto range = record.at("range").get<std::array<float, 2> >();
                auto slider = std::make_unique<GUISlider>(window, manager, range[0], range[1]);
                slider->setProgress(record.value("progress", 0.5f));
                gui = std::move(slider);
            } else gui = std::make_unique<GUIElement>(window, manager);
            const auto colour = record.at("colour").get<std::array<float, 3> >();
            gui->setColour({colour[0], colour[1], colour[2]});
            gui->setVisible(record.value("visible", true));
            if (record.contains("data")) {
                auto data = record.at("data").get<std::vector<uint8_t> >();
                if ((kind == "gui_button" && data.size() != sizeof(bool)) || (
                        kind == "gui_slider" && data.size() != sizeof(float)))
                    throw std::runtime_error("Invalid GUI payload size");
                gui->OwnData(data.data(), data.size());
            }
            if (record.contains("texture_asset")) gui->setTexture(
                manager->getAssetManager()->loadTexture(record.at("texture_asset").get<std::string>()).value().pointer);
            result = std::move(gui);
        } else if (kind.empty()) result = std::make_unique<GameObject>();
        else throw std::runtime_error("Unknown procedural object kind: " + kind);
        if (record.contains("procedural_materials")) {
            auto materials = ProceduralMaterials(*result);
            const auto &saved = record.at("procedural_materials");
            for (const auto &[name, material]: materials)
                if (saved.contains(name)) ReadMaterial(saved.at(name), *material, manager->getAssetManager());
        }
        return result;
    }

    void ResolveProceduralReferences(GameObject &root, const Json &records) {
        std::map<std::string, GameObject *> objects;
        for (auto *child: root.getAllChildren()) if (!child->IsGenerated()) objects[child->getName()] = child;
        for (const auto &record: records)
            if (record.contains("texture_source")) {
                const auto &reference = record.at("texture_source");
                auto *gui = dynamic_cast<GUIElement *>(objects.at(record.at("name").get<std::string>()));
                if (!gui) throw std::runtime_error("Texture source on a non-GUI object");
                auto outputs = Outputs(*objects.at(reference.at("object").get<std::string>()));
                gui->setTexture(outputs.at(reference.at("output").get<std::string>()));
            }
    }
}