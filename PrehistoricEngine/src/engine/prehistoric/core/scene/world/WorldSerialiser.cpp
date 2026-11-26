#include "Includes.hpp"
#include "WorldSerialiser.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"
#include "prehistoric/core/node/component/light/Light.h"
#include "prehistoric/core/node/component/VolumetricSphereComponent.h"
#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/application/Application.h"

#include <json.hpp>

namespace Prehistoric {
    static bool IsTerrainNode(Node *node) {
        for (; node; node = node->getParent())
            if (dynamic_cast<Terrain *>(node)) return true;
        return false;
    }

    void WorldSerialiser::SerialiseWorldJSON(const std::string &worldFile, Scene *scene) {
        GameObject *root = scene->getSceneRoot();
        sceneName = scene->getName();
        ListResources(root);
        const auto resources = manager->getAssetManager()->getIDMap();

        nlohmann::json file_json;
        file_json["name"] = sceneName;
        scene->ValidateSky();
        file_json["sky_schema"] = 1;
        file_json["temporal"] = {
            {"motion_blur", scene->temporal.motionBlur},
            {"antialiasing", scene->temporal.antialiasing}, {"shutter_angle", scene->temporal.shutterAngle},
            {"max_blur_pixels", scene->temporal.maxBlurPixels}, {"samples", scene->temporal.samples}
        };
        file_json["screen_space_reflections"] = scene->screenSpaceReflections;
        file_json["bloom_intensity"] = scene->bloomIntensity;
        const auto &ao = scene->ambientOcclusion;
        file_json["ambient_occlusion"] = {{"enabled", ao.enabled}, {"radius", ao.radius}, {"strength", ao.strength}};
        const auto &lens = scene->depthOfField;
        file_json["depth_of_field"] = {
            {"enabled", lens.enabled}, {"focus_distance", lens.focusDistance},
            {"aperture", lens.aperture}
        };
        if (const auto &pose = scene->getInitialCamera()) {
            file_json["initial_camera"] = {
                {"position", {pose->position.x, pose->position.y, pose->position.z}},
                {"forward", {pose->forward.x, pose->forward.y, pose->forward.z}},
                {"up", {pose->up.x, pose->up.y, pose->up.z}}
            };
            if (pose->bloom) file_json["initial_camera"]["bloom"] = *pose->bloom;
            if (pose->exposure) file_json["initial_camera"]["exposure"] = *pose->exposure;
        }

        {
            nlohmann::json directories;
            directories["models"] = directoryModels;
            directories["textures"] = directoryTextures;
            directories["configs"] = "";

            file_json["directories"] = directories;
        }

        {
            std::vector<nlohmann::json> models_list;
            for (auto &model: models) {
                nlohmann::json model_json;
                model_json["name"] = model.first;
                model_json["directory"] = model.second.directory;

                FrontFace frontFace = model.second.face;
                int front_face = 2;
                if (frontFace == FrontFace::CLOCKWISE) {
                    front_face = 0;
                } else if (frontFace == FrontFace::COUNTER_CLOCKWISE) {
                    front_face = 1;
                }
                model_json["front-face"] = front_face;

                models_list.push_back(model_json);
            }

            file_json["models"] = models_list;
        }

        {
            std::vector<nlohmann::json> textures_list;
            for (auto &texture: textures) {
                nlohmann::json texture_json;
                texture_json["name"] = texture.first;
                texture_json["directory"] = texture.first; //TODO: get the extension!!!

                textures_list.push_back(texture_json);
            }

            file_json["textures"] = textures_list;
        }

        {
            std::vector<nlohmann::json> materials_list;
            for (auto &material: materials) {
                nlohmann::json material_json;
                material_json["name"] = material.first;

                std::vector<nlohmann::json> contents_list;
                for (auto &tex: material.second->getTextures()) {
                    if (tex.first == "DEFAULT_TEX") continue;
                    nlohmann::json content;
                    content["name"] = tex.first;
                    content["type"] = "texture";

                    for (auto &elem: textures) {
                        if (elem.second.handle == tex.second.handle) {
                            content["value"] = elem.first;
                            break;
                        }
                    }

                    contents_list.push_back(content);
                }
                for (auto &vec4: material.second->getVector4fs()) {
                    nlohmann::json content;
                    content["name"] = vec4.first;
                    content["type"] = "vec4";

                    std::vector<float> value = {vec4.second.x, vec4.second.y, vec4.second.z, vec4.second.w};
                    content["value"] = value;

                    contents_list.push_back(content);
                }
                for (auto &vec3: material.second->getVector3fs()) {
                    nlohmann::json content;
                    content["name"] = vec3.first;
                    content["type"] = "vec3";

                    std::vector<float> value = {vec3.second.x, vec3.second.y, vec3.second.z};
                    content["value"] = value;

                    contents_list.push_back(content);
                }
                for (auto &vec2: material.second->getVector2fs()) {
                    nlohmann::json content;
                    content["name"] = vec2.first;
                    content["type"] = "vec2";

                    std::vector<float> value = {vec2.second.x, vec2.second.y};
                    content["value"] = value;

                    contents_list.push_back(content);
                }
                for (auto &fl: material.second->getFloats()) {
                    nlohmann::json content;
                    content["name"] = fl.first;
                    content["type"] = "float";
                    content["value"] = fl.second;

                    contents_list.push_back(content);
                }
                for (auto &in: material.second->getInts()) {
                    nlohmann::json content;
                    content["name"] = in.first;
                    content["type"] = "int";
                    content["value"] = in.second;

                    contents_list.push_back(content);
                }

                material_json["contents"] = contents_list;
                materials_list.push_back(material_json);
            }

            file_json["materials"] = materials_list;
        }

        {
            nlohmann::json root_json;
            std::vector<nlohmann::json> list_children;
            auto worldObjects = nlohmann::json::array();

            std::vector<GameObject *> children = root->getAllChildren();
            // Flat records retain explicit parent links and world-space transforms.
            for (auto child: children) {
                if (IsTerrainNode(child->getParent())) continue;
                if (const auto *terrain = dynamic_cast<const Terrain *>(child)) {
                    worldObjects.push_back({
                        {"name", child->getName()}, {"type", "terrain"}, {"config", terrain->GetConfigFile()}
                    });
                    continue;
                }

                if (const auto *environment = dynamic_cast<const EnvironmentMap *>(child)) {
                    const auto &transform = environment->getWorldTransform();
                    auto vec = [](const Vector3f &value) { return nlohmann::json::array({value.x, value.y, value.z}); };
                    worldObjects.push_back({
                        {"name", child->getName()}, {"type", "environment_map"},
                        {"asset", environment->GetAsset()}, {
                            "transform", {
                                {"position", vec(transform.getPosition())}, {"rotation", vec(transform.getRotation())},
                                {"scale", vec(transform.getScaling())}
                            }
                        }
                    });
                    continue;
                }
                if (const auto *atmosphere = dynamic_cast<const Atmosphere *>(child)) {
                    const auto sun = std::find_if(children.begin(), children.end(), [&](const GameObject *object) {
                        return atmosphere->getSun() && object->GetComponent<Light>() == atmosphere->getSun();
                    });
                    if (sun != children.end()) {
                        worldObjects.push_back({
                            {"name", child->getName()}, {"type", "scattering_atmosphere"},
                            {"sun", (*sun)->getName()}
                        });
                    }
                    continue;
                }
                nlohmann::json child_json;
                child_json["name"] = child->getName();
                child_json["enabled"] = child->isEnabled();
                if (child->getParent() && child->getParent() != root)
                    child_json["parent"] = child->getParent()->getName();

                {
                    nlohmann::json transform_json;

                    Transform &transform = child->getWorldTransform();
                    Vector3f position = transform.getPosition();
                    Vector3f rotation = transform.getRotation();
                    Vector3f scale = transform.getScaling();

                    std::vector<float> position_v = {position.x, position.y, position.z};
                    std::vector<float> rotation_v = {rotation.x, rotation.y, rotation.z};
                    std::vector<float> scale_v = {scale.x, scale.y, scale.z};

                    transform_json["position"] = position_v;
                    transform_json["rotation"] = rotation_v;
                    transform_json["scaling"] = scale_v;

                    child_json["transform"] = transform_json;
                }

                {
                    std::vector<nlohmann::json> components;

                    std::unordered_map<std::string, Component *> comps = child->getComponents();
                    for (auto elem: comps) {
                        Component *comp = elem.second;
                        nlohmann::json comp_json;

                        if (comp->getComponentType() == ComponentType::RendererComponent) {
                            RendererComponent *rendComp = (RendererComponent *) comp;

                            comp_json["type"] = "Renderer";
                            for (const auto &[index, overrideMaterial]: rendComp->getSubmeshOverrides()) {
                                const auto colour = overrideMaterial->getVector3f(COLOUR);
                                const auto mrot = overrideMaterial->getVector4f(MROT);
                                comp_json["submesh_overrides"].push_back({
                                    {"index", index},
                                    {"name", rendComp->getMeshBuffer()->getModel().getMesh(index).getName()},
                                    {"colour", {colour.x, colour.y, colour.z}},
                                    {"mrot", {mrot.x, mrot.y, mrot.z, mrot.w}},
                                    {"emission", overrideMaterial->getFloat(EMISSION)}
                                });
                            }

                            auto *graphics = dynamic_cast<GraphicsPipeline *>(rendComp->getPipeline());
                            if (!graphics) continue;
                            size_t vboHandle = graphics->getVertexBufferHandle().handle;

                            for (const auto &elem: models) {
                                if (elem.second.buffer.handle == vboHandle && elem.second.face == graphics->
                                    getFrontFace()) {
                                    comp_json["model"] = elem.first;
                                    break;
                                }
                            }

                            for (const auto &[name, resource]: resources) {
                                if (resource.type == ResourceType::Shader && resource.handle == rendComp->getPipeline()
                                    ->getShaderHandle().handle) {
                                    comp_json["shader"] = name;
                                    break;
                                }
                            }
                            for (const auto &[name, material]: materials) {
                                if (material.handle == rendComp->getMaterialHandle().handle) {
                                    comp_json["material"] = name;
                                    break;
                                }
                            }
                        } else if (comp->getComponentType() == ComponentType::LightComponent) {
                            Light *lightComp = (Light *) comp;

                            comp_json["type"] = "Light";

                            std::vector<float> colour_v = {
                                lightComp->getColour().x, lightComp->getColour().y, lightComp->getColour().z
                            };
                            comp_json["colour"] = colour_v;
                            comp_json["intensity"] = lightComp->getIntensity();
                            comp_json["range"] = lightComp->getRadius();
                            comp_json["illuminate"] = lightComp->illuminates();
                            comp_json["cast_shadows"] = lightComp->castShadows();
                        } else if (comp->getComponentType() == ComponentType::VolumetricSphereComponent) {
                            const auto *sphere = static_cast<const VolumetricSphereComponent *>(comp);
                            const auto &tint = sphere->getTint();
                            comp_json = {
                                {"type", VolumetricSphereComponent::Name},
                                {"radius", sphere->getRadius()},
                                {"density", sphere->getDensity()},
                                {"tint", {tint.x, tint.y, tint.z}},
                                {"anisotropy", sphere->getAnisotropy()}
                            };
                        }

                        if (!comp_json.is_null()) components.push_back(comp_json);
                    }

                    child_json["components"] = components;
                }

                list_children.push_back(child_json);
            }

            root_json["children"] = list_children;
            file_json["root"] = root_json;
            file_json["world_objects"] = worldObjects;
        }

        std::fstream file;
        file.open(worldFile.c_str(), std::ios::out);

        if (file.is_open()) {
            std::string result = file_json.dump(4);
            file.write(&result[0], result.size());

            file.flush();
            file.close();
        } else {
            PR_LOG_ERROR("Could not open world file %s!\n", worldFile.c_str());
        }
    }

    void WorldSerialiser::ListResources(GameObject *root) {
        std::vector<GameObject *> children = root->getAllChildren();
        //we assume that root doesn't have components, which is the way it should be

        //TODO: directory.model and .texture
        std::vector<std::string> modelLocations;
        std::vector<std::string> textureLocation;
        for (auto child: children) {
            if (IsTerrainNode(child)) continue;
            if (child->HasComponent<RendererComponent>()) {
                RendererComponent *renderer = child->GetComponent<RendererComponent>();

                auto *graphics = dynamic_cast<GraphicsPipeline *>(renderer->getPipeline());
                if (!graphics) continue;
                auto vbo = graphics->getVertexBufferHandle();

                MaterialHandle material = renderer->getMaterialHandle();
                std::unordered_map<std::string, TextureHandle> &matTextures = material->getTextures();

                //Store models
                std::unordered_map<std::string, Resource> ID_map = manager->getAssetManager()->getIDMap();
                for (auto &elem: ID_map) {
                    if (elem.second.type == ResourceType::VertexBuffer) {
                        if (elem.second.handle == vbo.handle) {
                            const auto face = graphics->getFrontFace();
                            auto name = elem.first;
                            if (auto it = models.find(name); it != models.end() && it->second.face != face)
                                name += "#" + std::to_string(static_cast<int>(face));
                            models.emplace(name, ModelReference{vbo, face, elem.first});
                            break;
                        }
                    }
                }

                //Store textures
                for (auto tex: matTextures) {
                    for (auto &elem: ID_map) {
                        if (elem.second.type == ResourceType::Texture) {
                            if (elem.second.handle == tex.second.handle) {
                                auto idx = textures.find(elem.first);
                                if (idx == textures.end()) {
                                    textures.insert(std::make_pair(elem.first, tex.second));
                                    break;
                                }
                            }
                        }
                    }
                }

                //Store material
                std::string matName = material->getName();
                if (matName.empty()) matName = "UnnamedMaterial" + std::to_string(material.handle);
                while (materials.count(matName) && materials.at(matName).handle != material.handle)
                    matName += "_" + std::to_string(material.handle);
                materials.emplace(matName, material);
            }
        }
    }
};