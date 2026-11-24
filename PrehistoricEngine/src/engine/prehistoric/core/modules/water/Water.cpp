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
        auto refractionDepth = man->storeTexture(GLTexture::Storage2D(refr_size, refr_size, 1, D32_LINEAR, Nearest, ClampToEdge, false));
        refractionFbo->addColourAttachment2D(refractionDepth.pointer);
        refractionFbo->Unbind();

        auto* surface = new Material(man, "Water");
        surface->addTexture("Dy", fft->getDyTexture());
        surface->addTexture("Dx", fft->getDxTexture());
        surface->addTexture("Dz", fft->getDzTexture());
        surface->addTexture("waterReflection", reflectionTexture);
        surface->addTexture("waterRefraction", refractionTexture);
        surface->addTexture("refractionDepth", refractionDepth);
        surface->addFloat("refractionDepthScale", 0);
        surface->addTexture("normalMap", normalMapRenderer->getNormalmap());
        surface->addTexture("dudvMap", dudv);
        surface->addFloat("waterHeight", position.y);
        surface->addFloat("motion", 0);
        surface->addInt("underwater", 0);
        material = manager->storeMaterial(surface);
        manager->addReference<Material>(material.handle);
        quadtree = new WaterQuadtree(window, camera, manager, material, position);
        lastLodPosition = camera->getPosition();
        AddChild("Quadtree", quadtree);
    }

    Water::~Water() {
        AssetManager *man = manager->getAssetManager();
        manager->removeReference<Material>(material.handle);
        man->removeReference<Texture>(reflectionTexture.handle);
        man->removeReference<Texture>(refractionTexture.handle);

        man->removeReference<Texture>(dudv.handle);
    }

    void Water::PreRender(Renderer *renderer) {
        camera = renderer->getCamera();
        const Vector3f offset = camera->getPosition() - position;
        const float halfExtent = __WaterConfig.scaleXZ * 0.5f;
        cameraUnderwater = offset.y < 0 && std::abs(offset.x) <= halfExtent && std::abs(offset.z) <= halfExtent;
        const float motion = float(Time::getTimeFromStartNanoseconds() * 1.0e-9 * __WaterConfig.wavemotion * 0.001);
        material->getFloats().at("motion") = motion;
        material->getInts().at("underwater") = cameraUnderwater;

        static_cast<GLRenderer*>(renderer)->RegisterWater(this);
        //FFT stuff
        fft->Render();
        normalMapRenderer->Render(fft->getDyTexture().pointer);

        //Water rendering
        UpdateQuadtree();
        Node::PreRender(renderer);
    }

    void Water::RenderCaptures(Renderer* renderer)
    {
        uint32_t width = window->getWidth();
        uint32_t height = window->getHeight();

        Camera* camera = renderer->getCamera();
        const Matrix4f view = camera->getViewMatrix();
        const Matrix4f projection = camera->getProjectionMatrix();
        const Matrix4f viewProjection = camera->getViewProjectionMatrix();
        const Vector3f eye = camera->getPosition();
        Matrix4f mirror = Matrix4f::Identity();
        mirror.m[5] = -1;
        mirror.m[13] = 2 * position.y;
        const float side = eye.y >= position.y ? 1.0f : -1.0f;

        auto capture = [&](Framebuffer& target, uint32_t size, bool reflection)
        {
            const Matrix4f captureView = reflection ? view * mirror : view;
            const Matrix4f vp = projection * captureView;
            const float keep = reflection ? side : -side;
            camera->setPosition(reflection ? Vector3f(eye.x, 2 * position.y - eye.y, eye.z) : eye);
            camera->setViewMatrix(captureView);
            camera->setProjectionMatrix(projection);
            camera->setViewProjectionMatrix(vp);
            __WaterConfig.stage = reflection ? WaterRenderStage::Reflection : WaterRenderStage::Refraction;
            target.Bind();
            target.Clear(0.0f);
            window->getSwapchain()->SetWindowSize(size, size);
            auto* gl = static_cast<GLRenderer*>(renderer);
            if (keep > 0) gl->DrawBackground(camera);

            // Map the retained water half-space to the OpenGL near clip plane.
            const Matrix4f inverse = vp.Invert();
            float farDistance = 0;
            for (float x : {-1.0f, 1.0f})
            {
                for (float y : {-1.0f, 1.0f})
                {
                    const Vector4f corner = inverse * Vector4f(x, y, 1, 1);
                    farDistance = (std::max)(farDistance, keep * (corner.y - position.y * corner.w));
                }
            }
            if (!reflection) material->getFloats().at("refractionDepthScale") = keep * farDistance;
            if (farDistance > 0.00001f)
            {
                Matrix4f clipped = vp;
                for (int column = 0; column < 4; ++column)
                {
                    const float plane = column == 1 ? keep : column == 3 ? -keep * position.y : 0;
                    clipped.m[column * 4 + 2] = 2 * plane / farDistance - vp.m[column * 4 + 3];
                }
                camera->setProjectionMatrix(clipped * captureView.Invert());
                camera->setViewProjectionMatrix(clipped);
                gl->DrawModelBatches(gl->getModelsWater());
            }
            target.Unbind();
        };

        capture(*reflectionFbo, refl_size, true);
        capture(*refractionFbo, refr_size, false);
        camera->setPosition(eye);
        camera->setViewMatrix(view);
        camera->setProjectionMatrix(projection);
        camera->setViewProjectionMatrix(viewProjection);
        window->getSwapchain()->SetWindowSize(width, height);
        __WaterConfig.stage = WaterRenderStage::Default;
    }

    void Water::UpdateQuadtree() {
        if (camera->getPosition() != lastLodPosition) {
            quadtree->UpdateQuadtree(camera);
            lastLodPosition = camera->getPosition();
        }
    }

}
