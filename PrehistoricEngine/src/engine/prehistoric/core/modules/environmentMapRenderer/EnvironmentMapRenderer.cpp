#include "Includes.hpp"
#include "EnvironmentMapRenderer.h"
#include "platform/opengl/rendering/renderStage/GLAtmospherePass.h"

#include "prehistoric/application/Application.h"

#include "prehistoric/core/modules/atmosphere/Atmosphere.h"

#include "platform/opengl/texture/GLTexture.h"
//#include "platform/vulkan/texture/VKTexture.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
//#include "platform/vulkan/rendering/framebuffer/VKFramebuffer.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"
//#include "platform/vulkan/rendering/pipeline/VKComputePipeline.h"

namespace Prehistoric {
    EnvironmentMapRenderer::EnvironmentMapRenderer(Window *window, AssembledAssetManager *manager,
                                                   GLAtmospherePass &atmospherePass)
        : atmospherePass(atmospherePass), window(window), manager(manager) {
        mapCache = "";

        AssetManager *man = manager->getAssetManager();

        //Create the cube mesh
        cubeBuffer = man->loadVertexBuffer(std::nullopt, "engine://models/cube.obj").value();
        cubeBuffer.pointer->setFrontFace(FrontFace::DOUBLE_SIDED);

        //Load the shaders and the pipelines
        environmentMapShader = man->storeShader(new GLEnvironmentMapShader());
        irradianceShader = man->storeShader(new GLIrradianceShader());
        prefilterShader = man->storeShader(new GLPrefilterShader());
        brdfIntegrateShader = man->storeShader(new GLBRDFIntegrateShader());
        environmentShader = man->storeShader(new GLEnvironmentShader());

        environmentMapPipeline = manager->createPipeline(PipelineTypeHashFlags::Compute, environmentMapShader);
        irradiancePipeline = manager->createPipeline(PipelineTypeHashFlags::Compute, irradianceShader);
        prefilterPipeline = manager->createPipeline(PipelineTypeHashFlags::Compute, prefilterShader);
        brdfIntegratePipeline = manager->createPipeline(PipelineTypeHashFlags::Compute, brdfIntegrateShader);
        backgroundPipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, environmentShader, cubeBuffer);

        manager->addReference<Pipeline>(environmentMapPipeline.handle);
        manager->addReference<Pipeline>(irradiancePipeline.handle);
        manager->addReference<Pipeline>(prefilterPipeline.handle);
        manager->addReference<Pipeline>(brdfIntegratePipeline.handle);
        manager->addReference<Pipeline>(backgroundPipeline.handle);

        //Create the framebuffer
        framebuffer = std::make_unique<GLFramebuffer>(window);

        //Create the matrices
        projectionMatrix = Matrix4f::PerspectiveProjection(90, 1, .1f, 100.0f);

        viewMatrices[0] = Matrix4f::View(Vector3f(-1, 0, 0), Vector3f(0, -1, 0));
        viewMatrices[1] = Matrix4f::View(Vector3f(1, 0, 0), Vector3f(0, -1, 0));
        viewMatrices[2] = Matrix4f::View(Vector3f(0, -1, 0), Vector3f(0, 0, -1));
        viewMatrices[3] = Matrix4f::View(Vector3f(0, 1, 0), Vector3f(0, 0, 1));
        viewMatrices[4] = Matrix4f::View(Vector3f(0, 0, 1), Vector3f(0, -1, 0));
        viewMatrices[5] = Matrix4f::View(Vector3f(0, 0, -1), Vector3f(0, -1, 0));

        //Creating the BRDF map
        constexpr uint32_t size = 256;
        brdfMap = man->storeTexture(GLTexture::Storage2D(size, size, 1, R16G16_LINEAR, Bilinear, ClampToEdge));
        manager->addReference<Texture>(brdfMap.handle);
        static_cast<GLComputePipeline *>(brdfIntegratePipeline.pointer)->setInvocationSize({size / 16, size / 16, 1});
        static_cast<GLComputePipeline *>(brdfIntegratePipeline.pointer)->addTextureBinding(
            0, brdfMap.pointer, (AccessMask) ComputeAccessFlags::WRITE_ONLY);

        //Render the brdf map, which is constant between differnet maps
        brdfIntegratePipeline->BindPipeline(nullptr);
        brdfIntegratePipeline->RenderPipeline();
        brdfIntegratePipeline->UnbindPipeline();

        brdfMap->Bind();
    }

    EnvironmentMapRenderer::~EnvironmentMapRenderer() {
        AssetManager *man = manager->getAssetManager();

        if (equirectangularMap.pointer != nullptr)
            man->removeReference<Texture>(equirectangularMap.handle);

        if (environmentMap.pointer) man->removeReference<Texture>(environmentMap.handle);
        if (irradianceMap.pointer) man->removeReference<Texture>(irradianceMap.handle);
        if (prefilterMap.pointer) man->removeReference<Texture>(prefilterMap.handle);

        man->removeReference<Texture>(brdfMap.handle);

        manager->removeReference<Pipeline>(environmentMapPipeline.handle);
        manager->removeReference<Pipeline>(irradiancePipeline.handle);
        manager->removeReference<Pipeline>(prefilterPipeline.handle);
        manager->removeReference<Pipeline>(brdfIntegratePipeline.handle);
        manager->removeReference<Pipeline>(backgroundPipeline.handle);
    }

    void EnvironmentMapRenderer::RequestBake(bool usePanorama) {
        panoramaOverride = usePanorama;
        requested = true;
    }

    void EnvironmentMapRenderer::UpdateEnvironment(const Atmosphere *atmosphere, bool includeSunDisk, const std::string& scenePanorama) {
        auto source = EnvironmentCaptureSource::FromScene(panoramaOverride ? nullptr : atmosphere,
                                                          __EngineConfig.rendererExposure,
                                                          panoramaOverride ? std::string{} : scenePanorama);
        source.includeSunDisk = !source.scattering || includeSunDisk;
        if (!requested && !capture.NeedsBake(source, __EnvironmentMapConfig.atmosphereRefreshSeconds)) return;
        GenerateEnvironmentMap(source);
        capture.Complete(source);
        requested = false;
    }

    void EnvironmentMapRenderer::GenerateEnvironmentMap(const EnvironmentCaptureSource &source) {
        AssetManager *man = manager->getAssetManager();
        const Atmosphere *atmosphere = source.atmosphere;
        TextureHandle previousEnvironment = environmentMap;
        TextureHandle previousIrradiance = irradianceMap;
        TextureHandle previousPrefilter = prefilterMap;

        auto *conversion = static_cast<GLComputePipeline *>(environmentMapPipeline.pointer);
        conversion->ResetBindings();
        static_cast<GLComputePipeline *>(irradiancePipeline.pointer)->ResetBindings();
        static_cast<GLComputePipeline *>(prefilterPipeline.pointer)->ResetBindings();
        if (!atmosphere && (!equirectangularMap.pointer || mapCache != source.panorama)) {
            auto next = man->loadTexture(source.panorama.empty() ? "engine://textures/default.png" : source.panorama,
                                         Bilinear, ClampToEdge).value();
            man->addReference<Texture>(next.handle);
            if (equirectangularMap.pointer) man->removeReference<Texture>(equirectangularMap.handle);
            equirectangularMap = next;
            mapCache = source.panorama;
        }

        {
            uint32_t size = __EnvironmentMapConfig.environmentMapResolution;

            if (atmosphere != nullptr) {
                environmentMap = man->storeTexture(
                    GLTexture::Storage3D(size, size, 1, R8G8B8A8_LINEAR, Trilinear, ClampToEdge, false));
            } else {
                environmentMap = man->storeTexture(
                    GLTexture::Storage3D(size, size, 1, R8G8B8A8_LINEAR, Trilinear, ClampToEdge, true));
            }

            {
                PR_PROFILE("Creating the textures");
                man->addReference<Texture>(environmentMap.handle);
                if (!atmosphere) {
                    conversion->setInvocationSize({size / 16, size / 16, 6});
                    conversion->addTextureBinding(0, environmentMap.pointer,
                                                  (AccessMask) ComputeAccessFlags::WRITE_ONLY);
                    conversion->addTextureBinding(1, equirectangularMap.pointer,
                                                  (AccessMask) ComputeAccessFlags::READ_ONLY);
                }

                size = __EnvironmentMapConfig.irradianceMapResolution;
                irradianceMap = man->storeTexture(
                    GLTexture::Storage3D(size, size, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
                man->addReference<Texture>(irradianceMap.handle);
                static_cast<GLComputePipeline *>(irradiancePipeline.pointer)->setInvocationSize({
                    size / 16, size / 16, 6
                });
                static_cast<GLComputePipeline *>(irradiancePipeline.pointer)->addTextureBinding(
                    0, irradianceMap.pointer, (AccessMask) ComputeAccessFlags::WRITE_ONLY);

                size = __EnvironmentMapConfig.prefilterMapResolution;
                prefilterMap = man->storeTexture(GLTexture::Storage3D(
                    size, size, __EnvironmentMapConfig.prefilterLevels, R8G8B8A8_LINEAR, Trilinear, ClampToEdge, true));
                man->addReference<Texture>(prefilterMap.handle);
            }
        }

        // Rendering the cube map
        {
            PR_PROFILE("Creating the cube map");
            if (atmosphere != nullptr) {
                uint32_t size = __EnvironmentMapConfig.environmentMapResolution;
                framebuffer->Bind();
                if (captureDepthSize != size) {
                    framebuffer->addDepthAttachment(size, size);
                    captureDepthSize = size;
                }
                window->getSwapchain()->SetWindowSize(size, size);

                for (uint32_t i = 0; i < 6; ++i) {
                    framebuffer->addColourAttachment3D(environmentMap.pointer, i);
                    framebuffer->Clear(0.0f);
                    atmospherePass.DrawCapture(source, viewMatrices[i], projectionMatrix);
                }

                window->getSwapchain()->SetWindowSize(window->getWidth(), window->getHeight());
                framebuffer->Unbind();
            } else {
                environmentMapPipeline->BindPipeline(nullptr);
                static_cast<GLEnvironmentMapShader *>(environmentMapShader.pointer)->UpdateUniforms(
                    Vector2f((float) equirectangularMap->getWidth(), (float) equirectangularMap->getHeight()));
                environmentMapPipeline->RenderPipeline();
                environmentMapPipeline->UnbindPipeline();
            }

            environmentMap->Bind();
            environmentMap->GenerateMipmaps();
        }

        //Rendering the diffuse irradiance map
        {
            PR_PROFILE("Creating the irradiance map");
            irradiancePipeline->BindPipeline(nullptr);
            static_cast<GLIrradianceShader *>(irradianceShader.pointer)->UpdateUniforms(environmentMap.pointer);
            irradiancePipeline->RenderPipeline();
            irradiancePipeline->UnbindPipeline();

            irradianceMap->Bind();
        }

        //Rendering the pre-filter enviroment map
        {
            PR_PROFILE("Creating the prefilter map");
            for (int level = 0; level < (int) __EnvironmentMapConfig.prefilterLevels; ++level) {
                int levelSize = (int) (__EnvironmentMapConfig.prefilterMapResolution * pow(0.5f, level));
                float roughness = (float) level / (__EnvironmentMapConfig.prefilterLevels - 1);

                if (level > 0)
                    static_cast<GLComputePipeline *>(prefilterPipeline.pointer)->removeTextureBinding(0);

                static_cast<GLComputePipeline *>(prefilterPipeline.pointer)->setInvocationSize({
                    (uint32_t) levelSize / 16, (uint32_t) levelSize / 16, 6
                });
                static_cast<GLComputePipeline *>(prefilterPipeline.pointer)->addTextureBinding(
                    0, prefilterMap.pointer, (AccessMask) ComputeAccessFlags::WRITE_ONLY, (size_t) level);

                prefilterPipeline->BindPipeline(nullptr);
                static_cast<GLPrefilterShader *>(prefilterShader.pointer)->UpdateUniforms(
                    environmentMap.pointer, roughness, (float) levelSize);
                prefilterPipeline->RenderPipeline();
            }
            prefilterPipeline->UnbindPipeline();

            prefilterMap->Bind();
        }

        if (previousEnvironment.pointer) {
            man->removeReference<Texture>(previousEnvironment.handle);
            man->removeReference<Texture>(previousIrradiance.handle);
            man->removeReference<Texture>(previousPrefilter.handle);
        }
    }

    void EnvironmentMapRenderer::RenderCube(Camera *camera, float lod) {
        backgroundPipeline->BindPipeline(nullptr);
        static_cast<GLEnvironmentShader *>(environmentShader.pointer)->UpdateUniforms(
            camera->getProjectionMatrix(), camera->getOriginalViewMatrix(), prefilterMap.pointer, lod);
        backgroundPipeline->RenderPipeline();
        backgroundPipeline->UnbindPipeline();
    }
};
