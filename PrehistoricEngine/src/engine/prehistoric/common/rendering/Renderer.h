#ifndef RENDERER_H
#define RENDERER_H

#include "prehistoric/common/rendering/DepthOfField.h"
#include "prehistoric/common/rendering/Temporal.h"
#include "prehistoric/common/rendering/AmbientOcclusion.h"
#include "prehistoric/common/framework/Window.h"
#include "prehistoric/core/node/movement/Camera.h"

#include "prehistoric/common/rendering/pipeline/Pipeline.h"

namespace Prehistoric {
    class RenderableComponent;
    class AssembledAssetManager;
    class Light;
    class Atmosphere;
    class EnvironmentMap;
    class Terrain;

    //The light structure used in the internal Forward+ renderer calculations
    struct InternalLight {
        Vector4f position;
        Vector4f colour;
        Vector4f intensity_radius;
    };

    struct VisibleIndex {
        int index;
    };

    struct VolumetricSphere {
        Vector3f center;
        float radius = 0.0f;
        float density = 0.0f; // Extinction per world unit at the center.
        Vector3f scatteringTint{1.0f}; // Fraction of extinction that scatters light, per channel.
        float anisotropy = 0.0f; // Positive values favor forward scattering.
    };

    using ModelBatches = std::unordered_map<Pipeline *, std::unordered_map<Material *, std::vector<RenderableComponent
        *> > >;

    class Renderer {
    public:
        Renderer(Window *window, Camera *camera, AssembledAssetManager *manager);

        virtual ~Renderer() {
        }

        virtual void PrepareRendering() = 0;

        virtual void EndRendering() = 0;

        virtual bool CanRenderFrame() const { return true; }

        virtual RenderExtent GetRenderExtent() const = 0;

        virtual std::vector<unsigned> GetSupportedSampleCounts() const = 0;

        virtual unsigned GetSampleCount() const = 0;

        bool RequestSampleCount(unsigned samples);

        virtual void RequestEnvironmentBake() = 0;

        virtual Vector4f GetVoxelVolume() const = 0; // Centre and width in world units.
        virtual void SetVoxelVolume(const Vector3f &center, float width, bool resetHistory = true) = 0;

        bool IsCameraInsideVoxelVolume() const;

        void FrameVoxelVolume();

        bool screenSpaceReflections = false;
        bool voxeliseTerrain = false;
        bool terrainReceivesGI = false;
        bool voxelFollowCamera = true;
        unsigned voxelCascadeCount = 4;
        int voxelDebugCascade = 0;
        bool voxelCascadeColours = false;
        uint64_t voxelCascadeBytes = 0;

        void UpdateVoxelFollow(unsigned resolution);

        struct VoxelUpdate {
            enum Mode { Reused, Scrolled, Rebuilt } mode = Reused;

            uint64_t dirtyCells = 0;
        } voxelUpdate;

        std::array<VoxelUpdate, 4> voxelCascadeUpdates{};

        float bloomIntensity = 0.05f;
        AmbientOcclusionSettings ambientOcclusion;
        DepthOfFieldSettings depthOfField;
        TemporalSettings temporal;
        virtual bool UseTemporalEffects() const { return temporal.active() && !wireframeMode; }
        TemporalHistory temporalHistory;
        bool temporalRendered = false;
        TemporalTimings temporalTimings;
        uint64_t submittedFrames = 0;
        uint64_t temporalBytes = 0;

        void ResetTemporalHistory() {
            temporalHistory.Reset();
            temporalRendered = false;
        }

        void CommitTemporalFrame();

        struct TemporalParameters {
            float reprojection[16];
            Vector4f jitter, settings, options, depthRow;
        };

        TemporalParameters GetTemporalParameters() const;

        bool sunShafts = true;
        float sunShaftStrength = 1.0f;
        bool sunFlare = true;
        float sunFlareStrength = 0.8f;
        float sunFlareSize = 1.0f;

        Vector4f GetSunFlare() const;

        Vector2f GetSunFlareFootprint() const;

        Texture *getFlareAtlas() const { return flareAtlas.get(); }

        float environmentPreviewLod = 0.0f;

        void Render();

        void SetSelection(const GameObject *object, std::optional<uint32_t> submesh) {
            selectedObject = object;
            selectedSubmesh = submesh;
        }

        bool hasSelection() const { return selectedObject != nullptr; }

        bool isSelected(const GameObject *object, uint32_t submesh) const {
            return selectedObject && object == selectedObject && (!selectedSubmesh || *selectedSubmesh == submesh);
        }

        bool isBloomEnabled() const { return bloomEnabled; }
        void setBloomEnabled(bool enabled) { bloomEnabled = enabled; }
        void ToggleBloom() { setBloomEnabled(!isBloomEnabled()); }

        virtual CommandBuffer *getDrawCommandBuffer() const { return nullptr; }

        virtual Texture *getOutputTexture() const { return nullptr; }

        void RegisterModel(RenderableComponent *renderable);

        void RegisterLight(Light *light);

        void RegisterTerrain(Terrain *terrain) { terrains.push_back(terrain); }
        const std::vector<Terrain *> &getTerrains() const { return terrains; }

        void RegisterAtmosphere(const Atmosphere &sky) {
            if (atmosphere || environmentObject) throw std::runtime_error("Multiple world sky sources submitted");
            atmosphere = &sky;
        }

        void RegisterEnvironmentMap(const EnvironmentMap &sky) {
            if (atmosphere || environmentObject) throw std::runtime_error("Multiple world sky sources submitted");
            environmentObject = &sky;
        }

        const Atmosphere *getAtmosphere() const { return atmosphere; }

        void RegisterVolumetricSphere(const VolumetricSphere &sphere) { volumetricSpheres.push_back(sphere); }
        const std::vector<VolumetricSphere> &getVolumetricSpheres() const { return volumetricSpheres; }

        void UnregisterModel(RenderableComponent *renderable);

        void UnregisterLight(Light *light);

        Window *getWindow() const { return window; }
        Camera *getCamera() const { return camera; }

        AssembledAssetManager *getAssetManager() const { return manager; }

        void setCamera(Camera *camera) {
            if (this->camera != camera) ResetTemporalHistory();
            this->camera = camera;
        }

        inline const std::vector<Light *> &getLights() const { return lights; }

        inline bool isWireframeMode() const { return wireframeMode; }
        inline bool isShadowMode() const { return renderingShadow; }
        inline void setWireframeMode(bool wire) { this->wireframeMode = wire; }

        const ModelBatches &getModels3D() const { return models_3d; }
        const ModelBatches &getModelsTransparency() const { return models_transparency; }

        const std::unordered_map<Pipeline *, std::vector<RenderableComponent *> > &getModels2D() const {
            return models_2d;
        }

        const ModelBatches &getModelsWater() const { return models_water; }

        template<typename Begin, typename MaterialBatch, typename Object, typename End>
        static void VisitBatches(const ModelBatches &batches, Begin begin, MaterialBatch materialBatch, Object object,
                                 End end) {
            for (const auto &[pipeline, materials]: batches) {
                begin(pipeline);
                for (const auto &[material, renderables]: materials) {
                    materialBatch(pipeline, material);
                    for (RenderableComponent *renderable: renderables) {
                        object(pipeline, renderable);
                    }
                }
                end(pipeline);
            }
        }

    protected:
        bool ApplyPendingSampleCount();

        std::optional<unsigned> pendingSampleCount;
        const EnvironmentMap *environmentObject = nullptr;

        virtual void UpdateEnvironmentLighting() = 0;

        virtual void RenderScene() = 0;

        virtual void RenderHDR() = 0;

        virtual void FinishFrame() = 0;

        const GameObject *selectedObject = nullptr;
        std::optional<uint32_t> selectedSubmesh;
        std::unique_ptr<Texture> flareAtlas;
        bool bloomEnabled = true;

        void ClearFrameSubmissions();

        void ResizePipelines(uint32_t width, uint32_t height);

        AssembledAssetManager *manager;

        Window *window;
        Camera *camera;

        //The rendering context
        ModelBatches models_3d;
        ModelBatches models_transparency;
        std::unordered_map<Pipeline *, std::vector<RenderableComponent *> > models_2d;

        ModelBatches models_water;

        std::vector<Light *> lights;
        std::vector<Terrain *> terrains;
        const Atmosphere *atmosphere = nullptr;
        std::vector<VolumetricSphere> volumetricSpheres;

        bool wireframeMode;
        bool renderingShadow = false;
    };
};

#endif
