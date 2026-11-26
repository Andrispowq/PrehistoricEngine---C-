#include "Includes.hpp"
#include "Scene.h"
#include "prehistoric/application/Application.h"

#include <filesystem>
#include <set>

namespace Prehistoric {
    Scene::Scene(const std::string &worldFile, Window *window, Camera *camera, AssembledAssetManager *manager)
        : window(window), camera(camera), manager(manager) {
        auto rootOwner = std::make_unique<GameObject>();
        sceneRoot = rootOwner.get();

        WorldLoader loader(window, manager);

        if (std::filesystem::path(worldFile).extension() == ".json") {
            loader.LoadWorldJSON(worldFile, sceneRoot);
        } else {
            loader.LoadWorld(worldFile, sceneRoot);
        }

        name = loader.sceneName;
        screenSpaceReflections = loader.screenSpaceReflections;
        bloomIntensity = loader.bloomIntensity;
        ambientOcclusion = loader.ambientOcclusion;
        depthOfField = loader.depthOfField;
        temporal = loader.temporal;
        initialCamera = loader.initialCamera;
        if (!GetSky())
            sceneRoot->AddChild("Environment", new EnvironmentMap(loader.environmentMap.empty()
                                                                      ? __EnvironmentMapConfig.environmentMapLocation
                                                                      : loader.environmentMap));
        else if (!loader.environmentMap.empty())
            throw std::runtime_error("World declares both an atmosphere/environment object and environment_map");
        ValidateSky();
        sceneRoot->sceneOwner = this;
        rootOwner.release();

        auto tex = loader.textures;
        for (auto elem: tex) {
            textures.push_back(elem.second);
        }
        auto mat = loader.materials;
        for (auto elem: mat) {
            materials.push_back(elem.second);
        }

        auto model = loader.models;
        for (auto elem: model) {
            models.push_back(elem.second);
        }
        auto sha = loader.shaders;
        for (auto elem: sha) {
            shaders.push_back(elem.second);
        }
        auto pip = loader.pipelines;
        for (auto elem: pip) {
            pipelines.push_back(elem.second);
        }
    }

    Scene::~Scene() {
        sceneRoot->sceneOwner = nullptr;
        Node *root = sceneRoot->getParent();
        if (root) {
            root->RemoveChild(sceneRoot);
        } else delete sceneRoot;
    }

    std::optional<RaycastHit> Scene::Raycast(const Vector3f &origin, const Vector3f &direction,
                                             float maxDistance) const {
        const float length = direction.length();
        if (!(length > 0) || !std::isfinite(length) || !(maxDistance > 0)) return std::nullopt;
        const auto ray = direction / length;
        std::optional<RaycastHit> hit;
        std::vector<Node *> pending{sceneRoot};
        while (!pending.empty()) {
            auto *node = pending.back();
            pending.pop_back();
            if (!node->isEnabled()) continue;
            for (const auto &child: node->getChildrenByID()) pending.push_back(child.second);
            auto *object = dynamic_cast<GameObject *>(node);
            auto *component = object ? object->GetComponent<RendererComponent>() : nullptr;
            auto *buffer = component ? component->getMeshBuffer() : nullptr;
            if (!buffer) continue;
            const auto *pipeline = dynamic_cast<GraphicsPipeline *>(component->getPipeline());
            if (!pipeline->drawsTriangleMesh()) continue;

            const auto &transform = object->getWorldTransform();
            const auto scale = transform.getScaling();
            if (scale.x == 0 || scale.y == 0 || scale.z == 0) continue;
            const auto inverse = transform.getTransformationMatrix().Invert();
            const auto localOrigin = (inverse * Vector4f(origin, 1)).xyz();
            const auto localDirection = (inverse * Vector4f(ray, 0)).xyz();
            auto face = pipeline->isBackfaceCulling() ? pipeline->getFrontFace() : FrontFace::DOUBLE_SIDED;
            if (face != FrontFace::DOUBLE_SIDED && ((scale.x < 0) ^ (scale.y < 0) ^ (scale.z < 0)))
                face = face == FrontFace::CLOCKWISE ? FrontFace::COUNTER_CLOCKWISE : FrontFace::CLOCKWISE;
            if (auto part = buffer->Raycast(localOrigin, localDirection, face, maxDistance))
                hit = RaycastHit{object, *part, maxDistance, origin + ray * maxDistance};
        }
        return hit;
    }

    bool Scene::IsSky(const Node *node) {
        return dynamic_cast<const Atmosphere *>(node) || dynamic_cast<const EnvironmentMap *>(node);
    }

    GameObject *Scene::GetSky() const {
        for (auto *object: sceneRoot->getAllChildren())
            if (IsSky(object)) return object;
        return nullptr;
    }

    void Scene::ValidateSky() const {
        unsigned count = 0;
        const auto objects = sceneRoot->getAllChildren();
        for (auto *object: objects) {
            if (!IsSky(object)) continue;
            ++count;
            for (Node *node = object; node != sceneRoot; node = node->getParent())
                if (!node->isEnabled()) throw std::runtime_error("World sky source must be enabled");
            if (auto *sky = dynamic_cast<Atmosphere *>(object)) {
                if (!sky->getSun() || std::find(objects.begin(), objects.end(), sky->getSun()->getParent()) == objects.
                    end())
                    throw std::runtime_error("Atmosphere requires a sun belonging to its world");
            }
        }
        if (count != 1) throw std::runtime_error("World must contain exactly one atmosphere or environment map");
    }

    void Scene::CheckAttach(const Node *node) const {
        if (replacingSky) return;
        if (IsSky(node)) throw std::runtime_error("Use ReplaceSky to change the world's sky source");
        for (const auto &[id, child]: node->getChildrenByID()) CheckAttach(child);
    }

    void Scene::CheckRemoval(const Node *node) const {
        if (replacingSky) return;
        const auto *sky = GetSky();
        const auto *atmosphere = dynamic_cast<const Atmosphere *>(sky);
        for (const Node *protectedNode: {
                 static_cast<const Node *>(sky),
                 atmosphere && atmosphere->getSun()
                     ? static_cast<const Node *>(atmosphere->getSun()->getParent())
                     : nullptr
             }) {
            for (auto *ancestor = protectedNode; ancestor; ancestor = ancestor->parent)
                if (ancestor == node) throw std::runtime_error(
                    "Replace the sky before removing or disabling it or its sun");
        }
    }

    void Scene::CheckComponentRemoval(const Component *component) const {
        const auto *sky = dynamic_cast<const Atmosphere *>(GetSky());
        if (sky && sky->getSun() == component)
            throw std::runtime_error("Cannot remove the atmosphere's sun light component");
    }

    void Scene::ReplaceSky(std::unique_ptr<GameObject> replacement) {
        if (!replacement || !IsSky(replacement.get()) || !replacement->isEnabled() ||
            !replacement->getChildrenByID().empty())
            throw std::runtime_error("Replacement must be one enabled sky object without children");
        if (auto *sky = dynamic_cast<Atmosphere *>(replacement.get())) {
            if (!sky->getSun() || sky->getSun()->getParent()->GetScene() != this)
                throw std::runtime_error("Replacement atmosphere requires a sun in this world");
        }
        auto *old = GetSky();
        if (old && !old->getChildrenByID().empty())
            throw std::runtime_error("Move the sky's children before replacing it");
        const std::string name = dynamic_cast<Atmosphere *>(replacement.get()) ? "Atmosphere" : "Environment";
        replacingSky = true;
        try {
            sceneRoot->AddChild(name, replacement.get());
            replacement.release();
            if (old) old->getParent()->RemoveChild(old);
        } catch (...) {
            replacingSky = false;
            throw;
        }
        replacingSky = false;
    }

    void Scene::ClearObjects() {
        auto replacement = std::make_unique<EnvironmentMap>(__EnvironmentMapConfig.environmentMapLocation);
        const auto previous = sceneRoot->getChildrenByID();
        replacingSky = true;
        try {
            sceneRoot->AddChild("Environment", replacement.get());
            replacement.release();
            for (auto [id, child]: previous) sceneRoot->RemoveChild(child);
        } catch (...) {
            replacingSky = false;
            throw;
        }
        replacingSky = false;
    }

    GameObject *Scene::Duplicate(GameObject &object) {
        if (object.GetScene() != this || &object == sceneRoot)
            throw std::runtime_error("Only objects belonging to this world can be duplicated");
        CheckAttach(&object);
        auto copy = object.Clone();
        std::set<std::string> names;
        for (auto *child: sceneRoot->getAllChildren()) names.insert(child->getName());
        auto unique = [&](const std::string &original) {
            const auto base = original + " copy";
            auto name = base;
            for (unsigned suffix = 2; names.count(name); ++suffix) name = base + " " + std::to_string(suffix);
            names.insert(name);
            return name;
        };
        const auto name = unique(object.getName());
        for (auto *child: copy->getAllChildren()) child->setName(unique(child->getName()));
        auto *result = copy.get();
        object.getParent()->AddChild(name, copy.get());
        copy.release();
        return result;
    }

    void Scene::AddGameObject(const std::string &key, GameObject *object) {
        sceneRoot->AddChild(key, object);
    }

    void Scene::RemoveGameObject(GameObject *object) {
        sceneRoot->RemoveChild(object);
    }
};