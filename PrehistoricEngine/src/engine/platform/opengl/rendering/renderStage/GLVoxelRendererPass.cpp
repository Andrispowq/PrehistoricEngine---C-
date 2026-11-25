#include "Includes.hpp"
#include "GLVoxelRendererPass.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/rendering/shaders/voxel/GLVoxelisationShader.h"
#include "prehistoric/common/buffer/MeshVertexBuffer.h"
#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/application/Application.h"

#include <array>
#include <optional>

namespace Prehistoric
{
    namespace
    {
        using MatrixState = std::array<float, 16>;

        MatrixState CaptureMatrix(const Matrix4f& matrix)
        {
            MatrixState state;
            std::copy_n(matrix.m, state.size(), state.begin());
            return state;
        }

        struct BatchState
        {
            const MeshVertexBuffer* buffer;
            std::vector<GLVoxelisationShader::Surface> surfaces;
            std::vector<MatrixState> transforms;

            bool operator==(const BatchState& other) const
            {
                return buffer == other.buffer && surfaces == other.surfaces && transforms == other.transforms;
            }
        };

        using SceneState = std::unordered_map<Pipeline*, std::unordered_map<Material*, BatchState>>;

        enum class CaptureMode { DebugColour, Radiance, ShadowCasters };

        SceneState CaptureBatches(const ModelBatches& batches, CaptureMode mode)
        {
            SceneState state;
            for (const auto& [pipeline, materials] : batches)
            {
                auto* vertices = static_cast<GLGraphicsPipeline*>(pipeline)->getVertexBuffer();
                if (vertices->getType() != VertexBufferType::MESH) continue;
                auto* mesh = static_cast<MeshVertexBuffer*>(vertices);
                auto& batchStates = state[pipeline];
                const auto submeshMaterials = mode == CaptureMode::ShadowCasters
                    ? std::vector<Material*>() : mesh->getModel().getMaterials();
                for (const auto& [material, objects] : materials)
                {
                    BatchState batch{mesh, {}, {}};
                    if (mode != CaptureMode::ShadowCasters)
                    {
                        for (uint32_t i = 0; i < mesh->getSubmeshCount(); ++i)
                        {
                            const int index = mesh->getModel().getMesh(i).getMaterialIndex();
                            auto surface = GLVoxelisationShader::GetSurface(index < 0 ? material : submeshMaterials[index]);
                            if (mode == CaptureMode::DebugColour) surface.emission = 0.0f;
                            batch.surfaces.push_back(surface);
                        }
                    }
                    batch.transforms.reserve(objects.size());
                    for (const auto* object : objects)
                        batch.transforms.push_back(CaptureMatrix(object->getParent()->getWorldTransform().getTransformationMatrix()));
                    std::sort(batch.transforms.begin(), batch.transforms.end());
                    batchStates.emplace(material, std::move(batch));
                }
            }
            return state;
        }

        struct LightingState
        {
            Vector3f direction;
            Vector3f radiance;
            MatrixState view;
            float fov;
            float aspect;
            float nearPlane;
            float farPlane;
            float sunAngularRadius;
            SceneState transparentCasters;

            bool operator==(const LightingState& other) const
            {
                return direction == other.direction && radiance == other.radiance && view == other.view
                    && fov == other.fov && aspect == other.aspect
                    && nearPlane == other.nearPlane && farPlane == other.farPlane
                    && sunAngularRadius == other.sunAngularRadius && transparentCasters == other.transparentCasters;
            }
        };

        struct BakeInputs
        {
            Vector3f center;
            float extent;
            SceneState meshes;
            std::optional<LightingState> lighting;

            bool operator==(const BakeInputs& other) const
            {
                return center == other.center && extent == other.extent
                    && meshes == other.meshes && lighting == other.lighting;
            }
        };
    }

    struct GLVoxelRendererPass::BakeCache
    {
        std::optional<BakeInputs> debug;
        std::optional<BakeInputs> radiance;
    };

    GLVoxelRendererPass::GLVoxelRendererPass(Renderer* renderer)
        : RenderStage(renderer), cache(std::make_unique<BakeCache>())
    {
    }

    GLVoxelRendererPass::~GLVoxelRendererPass()
    {
        if (voxelisationShader.pointer)
            manager->getAssetManager()->removeReference<Shader>(voxelisationShader.handle);
        if (framebuffer) glDeleteFramebuffers(1, &framebuffer);
        if (debugVolume) glDeleteTextures(1, &debugVolume);
        if (packedRadiance) glDeleteTextures(1, &packedRadiance);
        if (radianceVolume) glDeleteTextures(1, &radianceVolume);
    }

    bool GLVoxelRendererPass::Initialise()
    {
        if (initialisationAttempted) return ready;
        initialisationAttempted = true;

        AssetManager* assets = manager->getAssetManager();
        auto shader = assets->loadShader(ShaderName::Voxelisation);
        if (!shader) return false;
        voxelisationShader = *shader;
        assets->addReference<Shader>(voxelisationShader.handle);

        resolveShader = std::make_unique<GLShader>();
        compositeShader = std::make_unique<GLShader>();
        if (!resolveShader->AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxel_resolve_CS.glsl"), COMPUTE_SHADER)
            || !resolveShader->CompileShader()
            || !compositeShader->AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxel_view_CS.glsl"), COMPUTE_SHADER)
            || !compositeShader->CompileShader())
            return false;

        for (const char* uniform : {"debugView", "volumeMin", "volumeExtent", "giStrength", "inverseViewProjection"})
            compositeShader->AddUniform(uniform);

        glCreateFramebuffers(1, &framebuffer);
        glNamedFramebufferDrawBuffer(framebuffer, GL_NONE);
        glNamedFramebufferReadBuffer(framebuffer, GL_NONE);
        ready = true;
        return true;
    }

    void GLVoxelRendererPass::SetResolution(int size)
    {
        if (size == resolution) return;
        resolution = size;
        glDeleteTextures(1, &debugVolume);
        glDeleteTextures(1, &packedRadiance);
        glDeleteTextures(1, &radianceVolume);
        debugVolume = packedRadiance = radianceVolume = 0;
        cache->debug.reset();
        cache->radiance.reset();

        glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, size);
        glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, size);
    }

    GLuint GLVoxelRendererPass::GetPackedVolume(GLuint& texture)
    {
        if (!texture)
        {
            glCreateTextures(GL_TEXTURE_3D, 1, &texture);
            glTextureStorage3D(texture, 1, GL_R32UI, resolution, resolution, resolution);
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        return texture;
    }

    void GLVoxelRendererPass::AllocateRadianceVolume()
    {
        if (radianceVolume) return;
        const int size = resolution;
        int levels = 1;
        for (int width = size; width > 1; width /= 2) ++levels;
        glCreateTextures(GL_TEXTURE_3D, 1, &radianceVolume);
        glTextureStorage3D(radianceVolume, levels, GL_RGBA16F, size, size, size);
        glTextureParameteri(radianceVolume, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(radianceVolume, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(radianceVolume, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(radianceVolume, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(radianceVolume, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    void GLVoxelRendererPass::Render()
    {
        if (!IsSupported() || (!settings.debugView && !settings.globalIllumination) || !Initialise()) return;
        SetResolution(Resolutions[settings.resolutionIndex]);

        BakeInputs inputs{settings.center, settings.extent, CaptureBatches(renderer->getModels3D(),
            settings.debugView ? CaptureMode::DebugColour : CaptureMode::Radiance), std::nullopt};
        auto& previous = settings.debugView ? cache->debug : cache->radiance;
        if (!settings.debugView)
        {
            const auto* shadows = static_cast<GLRenderer*>(renderer)->getShadowDepthPass();
            if (const auto* sun = shadows->getSun())
            {
                const Vector3f radiance = sun->getColour() * sun->getIntensity();
                if (radiance.x > 0 || radiance.y > 0 || radiance.z > 0)
                {
                    // The shadow cascades used for injection follow the camera.
                    inputs.lighting = LightingState{
                        sun->getParent()->getWorldTransform().getPosition().normalise(), radiance,
                        CaptureMatrix(shadows->getShadowView()), renderer->getCamera()->getFov(),
                        static_cast<float>(window->getWidth()) / window->getHeight(),
                        __EngineConfig.rendererNearPlane, __EngineConfig.rendererFarPlane, __EngineConfig.shadowSunAngularRadius,
                        CaptureBatches(renderer->getModelsTransparency(), CaptureMode::ShadowCasters)
                    };
                }
            }
        }
        if (previous && *previous == inputs) return;

        PR_PROFILE(settings.debugView ? "Bake voxel debug volume" : "Bake voxel radiance");
        const GLuint packedVolume = GetPackedVolume(settings.debugView ? debugVolume : packedRadiance);

        const GLuint empty = 0;
        glClearTexImage(packedVolume, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &empty);
        glBindImageTexture(0, packedVolume, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

        GLint viewport[4], drawFramebuffer, polygonMode[2];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
        glGetIntegerv(GL_POLYGON_MODE, polygonMode);
        const bool culling = glIsEnabled(GL_CULL_FACE);
        const bool scissor = glIsEnabled(GL_SCISSOR_TEST);
        const bool rasterizerDiscard = glIsEnabled(GL_RASTERIZER_DISCARD);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, resolution, resolution);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_RASTERIZER_DISCARD);

        auto* shader = static_cast<GLVoxelisationShader*>(voxelisationShader.pointer);
        shader->Bind(nullptr);
        shader->SetUniformi("injectRadiance", !settings.debugView);
        if (settings.debugView) shader->SetUniformi("shadowMap", 1);
        else shader->UpdateGlobalUniforms(renderer);
        shader->SetUniform("volumeMin", settings.center - Vector3f(settings.extent * 0.5f));
        shader->SetUniformf("volumeExtent", settings.extent);

        for (const auto& [pipeline, batches] : renderer->getModels3D())
        {
            auto* buffer = static_cast<GLGraphicsPipeline*>(pipeline)->getVertexBuffer();
            if (buffer->getType() != VertexBufferType::MESH) continue;
            auto& model = static_cast<MeshVertexBuffer*>(buffer)->getModel();
            const auto materials = model.getMaterials();
            buffer->Bind(nullptr);
            for (const auto& [material, objects] : batches)
            {
                for (uint32_t i = 0; i < buffer->getSubmeshCount(); ++i)
                {
                    const int index = model.getMesh(i).getMaterialIndex();
                    shader->UpdateMaterialUniforms(index < 0 ? material : materials[index]);
                    for (const auto* object : objects)
                    {
                        shader->UpdateObjectUniforms(object->getParent());
                        buffer->Draw(nullptr, i);
                    }
                }
            }
            buffer->Unbind();
        }
        shader->Unbind();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glPolygonMode(GL_FRONT_AND_BACK, polygonMode[0]);
        if (culling) glEnable(GL_CULL_FACE);
        if (scissor) glEnable(GL_SCISSOR_TEST);
        if (rasterizerDiscard) glEnable(GL_RASTERIZER_DISCARD);

        if (settings.debugView)
        {
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
            glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
            previous = std::move(inputs);
            return;
        }

        AllocateRadianceVolume();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindImageTexture(1, radianceVolume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        resolveShader->Bind(nullptr);
        glDispatchCompute(resolution / 4, resolution / 4, resolution / 4);
        resolveShader->Unbind();
        glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        glGenerateTextureMipmap(radianceVolume);
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
        glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        previous = std::move(inputs);
    }

    void GLVoxelRendererPass::Composite(Texture* output) const
    {
        PR_PROFILE("Voxel composite");
        const auto* main = static_cast<GLRenderer*>(renderer)->getMainPass();
        if (settings.debugView) glBindTextureUnit(4, debugVolume);
        else
        {
            glBindTextureUnit(0, radianceVolume);
            main->getPositionMetallicImage()->Bind(1);
            main->getAlbedoRoughnessImage()->Bind(2);
            main->getNormalImage()->Bind(3);
        }

        compositeShader->Bind(nullptr);
        compositeShader->SetUniformi("debugView", settings.debugView);
        compositeShader->SetUniform("volumeMin", settings.center - Vector3f(settings.extent * 0.5f));
        compositeShader->SetUniformf("volumeExtent", settings.extent);
        compositeShader->SetUniformf("giStrength", settings.giStrength);
        if (settings.debugView)
        {
            const Camera* camera = renderer->getCamera();
            compositeShader->SetUniform("inverseViewProjection",
                (camera->getProjectionMatrix() * camera->getViewMatrix()).Invert());
        }

        glBindImageTexture(0, static_cast<GLTexture*>(output)->getTextureID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glDispatchCompute((window->getWidth() + 7) / 8, (window->getHeight() + 7) / 8, 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindTextureUnit(settings.debugView ? 4 : 0, 0);
        compositeShader->Unbind();
    }
}
