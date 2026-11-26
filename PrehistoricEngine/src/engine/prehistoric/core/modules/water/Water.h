#ifndef WATER_H
#define WATER_H

#include "prehistoric/core/node/Node.h"

#include "WaterQuadtree.h"

#include "prehistoric/core/modules/fft/FFT.h"
#include "prehistoric/core/modules/gpgpu/NormalMapRenderer.h"

namespace Prehistoric {
    class Renderer;
    struct WaterConfig;
    class Framebuffer;

    class VKWaterSimulation;

    class Water : public GameObject {
    public:
        static constexpr uint32_t captureSize = 1024;

        Water(Window *window, Camera *camera, AssembledAssetManager *manager, Vector3f position);

        virtual ~Water();

        Water(Window *window, Camera *camera, AssembledAssetManager *manager, Vector3f position,
              const WaterConfig &settings);

        float GetSimulationTime() const;

        void SetSimulationTime(float seconds);

        void PreRender(Renderer *renderer) override;

        void RenderCaptures(Renderer *renderer);

        void UpdateQuadtree();

        bool isCameraUnderwater() const { return cameraUnderwater; }

        Texture *getReflectionTexture() const { return reflectionTexture.pointer; }
        Texture *getRefractionTexture() const { return refractionTexture.pointer; }

        Water(const Water &) = delete;

        VKWaterSimulation &simulation() const { return *vkSimulation; }
        const std::vector<QuadPatch> &getLeaves() const { return leaves; }
        Material *GetSurfaceMaterial() const { return material.pointer; }
        float height() const { return worldTransform.getPosition().y; }
        float captureDepthScale = 0;

        std::unique_ptr<GameObject> Clone() const override;

        WaterConfig &GetSettings() const { return *settings; }

    private:
        std::unique_ptr<VKWaterSimulation> vkSimulation;
        std::vector<QuadPatch> leaves;

    private:
        std::unique_ptr<WaterConfig> settings;
        Window *window;
        Camera *camera;
        AssembledAssetManager *manager;

        WaterQuadtree *quadtree = nullptr;

        std::unique_ptr<FFT> fft;
        std::unique_ptr<NormalMapRenderer> normalMapRenderer;

        TextureHandle dudv;
        Vector3f position = 0;
        float timeOffset = 0;
        Vector3f lastLodPosition;

        MaterialHandle material;

        std::unique_ptr<Framebuffer> reflectionFbo;
        std::unique_ptr<Framebuffer> refractionFbo;

        TextureHandle reflectionTexture;
        TextureHandle refractionTexture;

        bool cameraUnderwater = false;
    };
}

#endif