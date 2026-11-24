#include "Includes.hpp"
#include "Water.h"
#include "WaterCapture.h"
#include "platform/vulkan/rendering/VKWaterSimulation.h"
#include "platform/vulkan/rendering/VKTerrain.h"
#include "platform/vulkan/rendering/VKRenderer.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/rendering/GLRenderer.h"

namespace Prehistoric {
    Water::Water(Window *window, Camera *camera, AssembledAssetManager *manager, Vector3f position)
        : window(window), camera(camera), manager(manager), position(position), fft{nullptr}, reflectionFbo{nullptr},
          refractionFbo{nullptr} {
        if (__FrameworkConfig.api == Vulkan)
        {
            auto* gpu = static_cast<VKDevice*>(window->getContext()->getDevices());
            VkPhysicalDeviceFeatures features{};
            vkGetPhysicalDeviceFeatures(gpu->getPhysicalDevice(), &features);
            if (!features.tessellationShader) return;
            auto* assets = manager->getAssetManager();
            vkSimulation = std::make_unique<VKWaterSimulation>(gpu, assets);
            auto* surface = new Material(assets, "Water");
            surface->addTexture("dudvMap", assets->loadTexture("engine://textures/water/dudv1.jpg", Bilinear, Repeat).value());
            material = manager->storeMaterial(surface);
            auto vertices = CreateVulkanQuadPatch(window, manager);
            vertices->setFrontFace(FrontFace::DOUBLE_SIDED);
            auto shader = assets->loadShader(ShaderName::Water).value();
            auto pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vertices);
            auto* component = new RendererComponent(window, manager, pipeline, material);
            component->setReflection(false);
            AddComponent(RENDERER_COMPONENT, component);
            SetPosition(position - Vector3f(__WaterConfig.scaleXZ/2,0,__WaterConfig.scaleXZ/2));
            SetScale({__WaterConfig.scaleXZ,1,__WaterConfig.scaleXZ});
            quadtree = new WaterQuadtree(window,camera,manager,material,position);
            AddChild("Quadtree",quadtree);
            quadtree->CollectLeaves(leaves);
            lastLodPosition = camera->getPosition();
            return;
        }
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
        if (!fft) return;
        AssetManager *man = manager->getAssetManager();
        manager->removeReference<Material>(material.handle);
        man->removeReference<Texture>(reflectionTexture.handle);
        man->removeReference<Texture>(refractionTexture.handle);

        man->removeReference<Texture>(dudv.handle);
    }

    void Water::PreRender(Renderer *renderer) {
        if (!enabled || (!fft && !vkSimulation)) return;
        camera = renderer->getCamera();
        const Vector3f offset = camera->getPosition() - position;
        const float halfExtent = __WaterConfig.scaleXZ * 0.5f;
        cameraUnderwater = offset.y < 0 && std::abs(offset.x) <= halfExtent && std::abs(offset.z) <= halfExtent;
        if (vkSimulation)
        {
            UpdateQuadtree();
            static_cast<VKRenderer*>(renderer)->RegisterWater(this);
            GetComponent<RendererComponent>()->PreRender(renderer);
            return;
        }
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
        auto capture = [&](Framebuffer& target, uint32_t size, bool reflection)
        {
            const auto pass = MakeWaterCapture(view, projection, eye, position.y, reflection, false);
            camera->setPosition(pass.eye);
            camera->setViewMatrix(pass.view);
            camera->setProjectionMatrix(projection);
            camera->setViewProjectionMatrix(projection * pass.view);
            __WaterConfig.stage = reflection ? WaterRenderStage::Reflection : WaterRenderStage::Refraction;
            target.Bind();
            target.Clear(0.0f);
            window->getSwapchain()->SetWindowSize(size, size);
            auto* gl = static_cast<GLRenderer*>(renderer);
            if (pass.sky) gl->DrawBackground(camera);
            if (!reflection) material->getFloats().at("refractionDepthScale") = pass.depthScale;
            if (pass.visible)
            {
                camera->setProjectionMatrix(pass.clippedProjection);
                camera->setViewProjectionMatrix(pass.clippedProjection * pass.view);
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
        if (quadtree && camera->getPosition() != lastLodPosition) {
            quadtree->UpdateQuadtree(camera);
            if (vkSimulation) quadtree->CollectLeaves(leaves);
            lastLodPosition = camera->getPosition();
        }
    }

}
