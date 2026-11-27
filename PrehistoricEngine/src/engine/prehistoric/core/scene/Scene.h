#ifndef SCENE_H
#define SCENE_H

#include "prehistoric/common/rendering/Temporal.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"
#include "world/WorldLoader.h"

#include "prehistoric/core/modules/terrain/Terrain.h"

#include "platform/vulkan/rendering/pipeline/VKPipeline.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"
#include "platform/vulkan/texture/VkTexture.h"
#include "platform/vulkan/rendering/shaders/pbr/VKPBRShader.h"

#include "prehistoric/core/modules/gui/GUIElement.h"
#include "prehistoric/core/modules/gui/button/GUIButton.h"
#include "prehistoric/core/modules/gui/slider/GUISlider.h"

#include "prehistoric/core/modules/atmosphere/Atmosphere.h"
#include "prehistoric/core/modules/atmosphere/EnvironmentMap.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/core/node/component/audio/AudioComponent.h"

namespace Prehistoric {
    struct RaycastHit {
        GameObject *object;
        uint32_t submesh;
        float distance;
        Vector3f position;
    };

    class Scene {
    public:
        Scene(const std::string &worldFile, Window *window, Camera *camera, AssembledAssetManager *manager);

        virtual ~Scene();

        std::optional<RaycastHit> Raycast(const Vector3f &origin, const Vector3f &direction, float maxDistance) const;

        void AddGameObject(const std::string &name, GameObject *object);

        void RemoveGameObject(GameObject *object);

        std::string getName() const { return name; }
        GameObject *getSceneRoot() const { return sceneRoot; }
        bool screenSpaceReflections = false;
        bool voxeliseTerrain = false;
        bool terrainReceivesGI = false;
        float bloomIntensity = 0.35f;
        AmbientOcclusionSettings ambientOcclusion;
        DepthOfFieldSettings depthOfField;
        TemporalSettings temporal;
        const std::optional<WorldCameraPose> &getInitialCamera() const { return initialCamera; }

        GameObject *GetSky() const;

        void ValidateSky() const;

        void CheckAttach(const Node *node) const;

        void CheckRemoval(const Node *node) const;

        void CheckComponentRemoval(const Component *component) const;

        void ReplaceSky(std::unique_ptr<GameObject> replacement);

        void ClearObjects();

        GameObject *Duplicate(GameObject &object);

        static bool IsSky(const Node *node);

    protected:
        bool replacingSky = false;
        std::string name = "UnnamedScene";
        std::optional<WorldCameraPose> initialCamera;

        GameObject *sceneRoot;
        Window *window;
        Camera *camera;
        AssembledAssetManager *manager;

        std::vector<TextureHandle> textures;
        std::vector<MaterialHandle> materials;

        std::vector<VertexBufferHandle> models;
        std::vector<ShaderHandle> shaders;
        std::vector<PipelineHandle> pipelines;
    };
};

#endif