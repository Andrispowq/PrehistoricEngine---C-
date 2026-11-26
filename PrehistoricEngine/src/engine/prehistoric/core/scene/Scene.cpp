#include "Includes.hpp"
#include "Scene.h"

#include <filesystem>

namespace Prehistoric {
    Scene::Scene(const std::string &worldFile, Window *window, Camera *camera, AssembledAssetManager *manager)
        : window(window), camera(camera), manager(manager) {
        sceneRoot = new GameObject();

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
        environmentMap = loader.environmentMap;

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
        Node *root = sceneRoot->getParent();
        if (root) {
            root->RemoveChild(sceneRoot);
        }
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

    void Scene::AddGameObject(const std::string &key, GameObject *object) {
        sceneRoot->AddChild(key, object);
    }

    void Scene::RemoveGameObject(GameObject *object) {
        sceneRoot->RemoveChild(object);
    }
};