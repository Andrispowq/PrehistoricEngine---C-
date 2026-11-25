#include "Includes.hpp"
#include "GLVoxelRendererPass.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/rendering/shaders/voxel/GLVoxelisationShader.h"
#include "prehistoric/common/buffer/MeshVertexBuffer.h"
#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/application/Application.h"

#include <array>
#include <chrono>
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

        using BakeClock = std::chrono::steady_clock;
        constexpr int VoxelShadowSize = 1024;
        constexpr float CubeDiagonal = 1.7320508f;

        class VoxelShadowShader final : public GLShader
        {
        public:
            void UpdateObjectUniforms(GameObject* object, uint32_t) const override
            {
                SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix());
            }
        };

        struct LightingState
        {
            const Light* source = nullptr;
            Vector3f direction = Vector3f(0, 1, 0);
            Vector3f radiance = Vector3f(0);

            bool operator==(const LightingState& other) const
            {
                return source == other.source && direction == other.direction && radiance == other.radiance;
            }
        };

        LightingState CaptureSun(const Renderer& renderer)
        {
            const auto* sun = GLShadowDepthPass::FindSun(renderer);
            if (sun)
            {
                const Vector3f radiance = sun->getColour() * sun->getIntensity();
                if (radiance.x > 0 || radiance.y > 0 || radiance.z > 0)
                    return {sun, sun->getParent()->getWorldTransform().getPosition().normalise(), radiance};
            }
            return {};
        }

        struct BakeInputs
        {
            Vector3f center;
            float extent;
            SceneState meshes;
            SceneState transparentCasters;

            bool operator==(const BakeInputs& other) const
            {
                return center == other.center && extent == other.extent
                    && meshes == other.meshes && transparentCasters == other.transparentCasters;
            }
        };
    }

    struct GLVoxelRendererPass::BakeCache
    {
        std::optional<BakeInputs> debug;
        std::optional<BakeInputs> radiance;
        LightingState lighting;
        BakeClock::time_point completed;
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
        if (shadowFramebuffer) glDeleteFramebuffers(1, &shadowFramebuffer);
        if (shadowDepth) glDeleteTextures(1, &shadowDepth);
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

        auto loadCompute = [](const char* path) -> std::unique_ptr<GLShader>
        {
            auto program = std::make_unique<GLShader>();
            if (!program->AddShader(ResourceLoader::LoadShaderGL(path), COMPUTE_SHADER) || !program->CompileShader())
                return nullptr;
            return program;
        };

        resolveShader = loadCompute("opengl/voxel/voxel_resolve_CS.glsl");
        mipShader = loadCompute("opengl/voxel/voxel_mip_CS.glsl");
        compositeShader = loadCompute("opengl/voxel/voxel_view_CS.glsl");
        if (!resolveShader || !mipShader || !compositeShader) return false;

        shadowShader = std::make_unique<VoxelShadowShader>();
        if (!shadowShader->AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxel_shadow_VS.glsl"), VERTEX_SHADER)
            || !shadowShader->AddShader(ResourceLoader::LoadShaderGL("opengl/shadow/depth_pass_FS.glsl"), FRAGMENT_SHADER)
            || !shadowShader->CompileShader())
            return false;
        shadowShader->AddUniform("m_transform");
        shadowShader->AddUniform("lightViewProjection");

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

    void GLVoxelRendererPass::GenerateRadianceMips() const
    {
        PR_PROFILE("Generate voxel radiance mips");
        mipShader->Bind(nullptr);
        int size = resolution;
        for (int level = 1; size > 1; ++level)
        {
            size /= 2;
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glBindImageTexture(0, radianceVolume, level - 1, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
            glBindImageTexture(1, radianceVolume, level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            glDispatchCompute((size + 3) / 4, (size + 3) / 4, (size + 3) / 4);
        }
        mipShader->Unbind();
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    Matrix4f GLVoxelRendererPass::RenderSunShadow(const Vector3f& direction)
    {
        PR_PROFILE("Bake voxel sun shadow");
        if (!shadowDepth)
        {
            glCreateTextures(GL_TEXTURE_2D, 1, &shadowDepth);
            glTextureStorage2D(shadowDepth, 1, GL_DEPTH_COMPONENT32F, VoxelShadowSize, VoxelShadowSize);
            glTextureParameteri(shadowDepth, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(shadowDepth, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(shadowDepth, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(shadowDepth, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(shadowDepth, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(shadowDepth, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

            glCreateFramebuffers(1, &shadowFramebuffer);
            glNamedFramebufferTexture(shadowFramebuffer, GL_DEPTH_ATTACHMENT, shadowDepth, 0);
            glNamedFramebufferDrawBuffer(shadowFramebuffer, GL_NONE);
            glNamedFramebufferReadBuffer(shadowFramebuffer, GL_NONE);
        }

        const Vector3f up = std::abs(direction.y) > 0.9999f ? Vector3f(0, 0, 1) : Vector3f(0, 1, 0);
        const Vector3f lightUp = direction.cross(up).normalise().cross(direction).normalise();
        const float radius = settings.extent * CubeDiagonal * 0.5f;
        // Include casters on the light-facing side of the cube.
        const auto projection = Matrix4f::OrthographicProjection(-radius, radius, -radius, radius,
                                                                 -(radius + settings.extent), radius);
        const auto lightSpace = projection * Matrix4f::View(direction, lightUp)
                             * Matrix4f::Translation(settings.center * -1.0f);

        const bool depthTest = glIsEnabled(GL_DEPTH_TEST);
        GLboolean depthWrite;
        GLint depthFunction;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunction);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowFramebuffer);
        glViewport(0, 0, VoxelShadowSize, VoxelShadowSize);
        const GLfloat farDepth = 1.0f;
        glClearBufferfv(GL_DEPTH, 0, &farDepth);
        shadowShader->Bind(nullptr);
        shadowShader->SetUniform("lightViewProjection", lightSpace);
        GLDepthPass::DrawBatches(renderer->getModels3D(), *shadowShader);
        GLDepthPass::DrawBatches(renderer->getModelsTransparency(), *shadowShader);
        shadowShader->Unbind();

        glDepthMask(depthWrite);
        glDepthFunc(depthFunction);
        if (!depthTest) glDisable(GL_DEPTH_TEST);
        return lightSpace;
    }

    void GLVoxelRendererPass::Render()
    {
        if (!IsSupported() || (!settings.debugView && !settings.globalIllumination) || !Initialise()) return;
        SetResolution(Resolutions[settings.resolutionIndex]);

        const auto lighting = settings.debugView ? LightingState{} : CaptureSun(*renderer);
        BakeInputs inputs{settings.center, settings.extent, CaptureBatches(renderer->getModels3D(),
            settings.debugView ? CaptureMode::DebugColour : CaptureMode::Radiance), {}};
        if (lighting.source)
            inputs.transparentCasters = CaptureBatches(renderer->getModelsTransparency(), CaptureMode::ShadowCasters);

        auto& previous = settings.debugView ? cache->debug : cache->radiance;
        if (previous && *previous == inputs)
        {
            if (settings.debugView || lighting == cache->lighting) return;
            const auto interval = std::chrono::duration<float>(__EnvironmentMapConfig.atmosphereRefreshSeconds);
            if (lighting.source == cache->lighting.source && BakeClock::now() - cache->completed < interval) return;
        }

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
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_RASTERIZER_DISCARD);

        const auto lightSpace = lighting.source ? RenderSunShadow(lighting.direction) : Matrix4f::Identity();
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, resolution, resolution);

        auto* shader = static_cast<GLVoxelisationShader*>(voxelisationShader.pointer);
        shader->Bind(nullptr);
        shader->SetUniformi("injectRadiance", !settings.debugView);
        glBindTextureUnit(1, shadowDepth);
        if (!settings.debugView)
        {
            shader->SetUniform("sunDirection", lighting.direction);
            shader->SetUniform("sunRadiance", lighting.radiance);
            shader->SetUniform("lightViewProjection", lightSpace);
            shader->SetUniformf("shadowBias", 1.5f * CubeDiagonal / (VoxelShadowSize * (CubeDiagonal + 1.0f)));
        }
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
        glBindTextureUnit(1, 0);

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
        GenerateRadianceMips();
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        previous = std::move(inputs);
        cache->lighting = lighting;
        cache->completed = BakeClock::now();
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
