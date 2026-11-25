#ifndef WATER_H
#define WATER_H

#include "prehistoric/core/node/Node.h"

#include "WaterQuadtree.h"

#include "prehistoric/core/modules/fft/FFT.h"
#include "prehistoric/core/modules/gpgpu/NormalMapRenderer.h"

namespace Prehistoric {
    class Renderer;
    class Framebuffer;

    class VKWaterSimulation;

    class Water : public GameObject {
    public:
        static constexpr uint32_t captureSize = 1024;

        Water(Window *window, Camera *camera, AssembledAssetManager *manager, Vector3f position);

        virtual ~Water();

        void PreRender(Renderer *renderer) override;

        void RenderCaptures(Renderer *renderer);

        void UpdateQuadtree();

        bool isCameraUnderwater() const { return cameraUnderwater; }

        Texture *getReflectionTexture() const { return reflectionTexture.pointer; }
        Texture *getRefractionTexture() const { return refractionTexture.pointer; }

        Water(const Water &) = delete;

        VKWaterSimulation &simulation() const { return *vkSimulation; }
        const std::vector<QuadPatch> &getLeaves() const { return leaves; }
        float height() const { return position.y; }
        float captureDepthScale = 0;

    private:
        std::unique_ptr<VKWaterSimulation> vkSimulation;
        std::vector<QuadPatch> leaves;

    private:
        Window *window;
        Camera *camera;
        AssembledAssetManager *manager;

        WaterQuadtree *quadtree = nullptr;

        std::unique_ptr<FFT> fft;
        std::unique_ptr<NormalMapRenderer> normalMapRenderer;

        TextureHandle dudv;
        Vector3f position = 0;
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