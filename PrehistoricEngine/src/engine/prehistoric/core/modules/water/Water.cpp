#include "Includes.hpp"
#include "Water.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/rendering/GLRenderer.h"

namespace Prehistoric {
    Water::Water(Window *window, Camera *camera, AssembledAssetManager *manager, Vector3f position)
        : window(window), camera(camera), manager(manager), position(position), fft{nullptr}, reflectionFbo{nullptr},
          refractionFbo{nullptr} {
        fft = std::make_unique<FFT>(manager, window, &__WaterConfig);
        normalMapRenderer = std::make_unique<NormalMapRenderer>(window, manager, __WaterConfig.normalStrength,
                                                                __WaterConfig.fftResolution);

        AssetManager *man = manager->getAssetManager();
        dudv = man->loadTexture("engine://textures/water/dudv1.jpg", Bilinear, Repeat).value();
        man->addReference<Texture>(dudv.handle);

        //Create framebuffers
        reflectionTexture = man->storeTexture(GLTexture::Storage2D(refl_size, refl_size, 1, R8G8B8A8_LINEAR, Bilinear));
        refractionTexture = man->storeTexture(GLTexture::Storage2D(refr_size, refr_size, 1, R8G8B8A8_LINEAR, Bilinear));
        man->addReference<Texture>(reflectionTexture.handle);
        man->addReference<Texture>(refractionTexture.handle);

        reflectionFbo = std::make_unique<GLFramebuffer>(window);
        reflectionFbo->Bind();
        reflectionFbo->addColourAttachment2D(reflectionTexture.pointer);
        reflectionFbo->addDepthAttachment(refl_size, refl_size);
        reflectionFbo->Unbind();

        refractionFbo = std::make_unique<GLFramebuffer>(window);
        refractionFbo->Bind();
        refractionFbo->addColourAttachment2D(refractionTexture.pointer);
        refractionFbo->addDepthAttachment(refr_size, refr_size);
        refractionFbo->Unbind();

        //data
        texData = new TexturesData();
        texData->Dy = fft->getDyTexture();
        texData->Dx = fft->getDxTexture();
        texData->Dz = fft->getDzTexture();

        texData->waterHeight = position.y;

        texData->waterReflection = reflectionTexture.pointer;
        texData->waterRefraction = refractionTexture.pointer;
        texData->dudvMap = dudv.pointer;

        //Add water
        quadtree = new WaterQuadtree(window, camera, manager, texData, position);
        AddChild("Quadtree", quadtree);
    }

    Water::~Water() {
        AssetManager *man = manager->getAssetManager();
        man->removeReference<Texture>(reflectionTexture.handle);
        man->removeReference<Texture>(refractionTexture.handle);

        man->removeReference<Texture>(dudv.handle);
    }

    void Water::PreRender(Renderer *renderer) {
        //Very bad
        cameraUnderwater = renderer->getCamera()->getPosition().y < position.y;
        //glEnable(GL_CLIP_DISTANCE0);

        __WaterConfig.distortionDelta += distortionDelta;
        motion += __WaterConfig.wavemotion;
        texData->motion = motion;
        texData->underwater = cameraUnderwater;

        static_cast<GLRenderer*>(renderer)->RegisterWater(this);
        //FFT stuff
        fft->Render();
        normalMapRenderer->Render(fft->getDyTexture());

        texData->normalMap = normalMapRenderer->getNormalmap().pointer;

        //Water rendering
        UpdateQuadtree();
        Node::PreRender(renderer);
    }

    void Water::RenderCaptures(Renderer* renderer)
    {
        uint32_t width = window->getWidth();
        uint32_t height = window->getHeight();

        //flipping upside down
        Matrix4f refl = Matrix4f::Identity();
        refl.m[1 * 4 + 1] = -1;
        refl.m[3 * 4 + 1] = 2 * position.y;

        Camera *camera = renderer->getCamera();

        Matrix4f view = camera->getViewMatrix();
        Matrix4f refl_view = refl * view;
        camera->setViewMatrix(refl_view);
        camera->setViewProjectionMatrix(camera->getProjectionMatrix() * refl_view);

        //reflection
        reflectionFbo->Bind();
        reflectionFbo->Clear(0.0f);
        window->getSwapchain()->SetWindowSize(refl_size, refl_size);
        glFrontFace(GL_CCW);
        __WaterConfig.stage = WaterRenderStage::Reflection;

        if (!cameraUnderwater) {
            RenderObjects(renderer);
        }

        glFinish();
        glFrontFace(GL_CW);
        reflectionFbo->Unbind();

        //flipping back
        camera->setViewMatrix(view);
        camera->setViewProjectionMatrix(camera->getProjectionMatrix() * view);

        //refraction
        refractionFbo->Bind();
        refractionFbo->Clear(0.0f);
        window->getSwapchain()->SetWindowSize(refr_size, refr_size);
        __WaterConfig.stage = WaterRenderStage::Refraction;

        RenderObjects(renderer);

        glFinish();

        refractionFbo->Unbind();
        window->getSwapchain()->SetWindowSize(width, height);
        //glDisable(GL_CLIP_DISTANCE0);
        __WaterConfig.stage = WaterRenderStage::Default;

    }

    void Water::UpdateQuadtree() {
        if (camera->isChanged()) {
            quadtree->UpdateQuadtree();
        }
    }

    void Water::RenderObjects(Renderer *renderer) const {
        GLRenderer* rend = static_cast<GLRenderer*>(renderer);
        if (const Atmosphere* sky = rend->getAtmosphere())
        {
            rend->getAtmospherePass().DrawScene(*sky, *camera);
        }
        else
        {
            EnvironmentMapRenderer::instance->RenderCube(camera);
        }

        rend->DrawModelBatches(rend->getModelsWater());
    }
}
