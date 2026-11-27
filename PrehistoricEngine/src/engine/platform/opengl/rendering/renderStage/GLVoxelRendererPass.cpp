#include "Includes.hpp"
#include "GLVoxelRendererPass.h"
#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/rendering/shaders/voxel/GLVoxelisationShader.h"
#include "prehistoric/common/buffer/MeshVertexBuffer.h"
#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/application/Application.h"

#include "prehistoric/common/rendering/TerrainVoxel.h"
#include "prehistoric/common/rendering/VoxelScroll.h"
#include <array>
#include <chrono>
#include <optional>

namespace Prehistoric {
    namespace {
        using MatrixState = std::array<float, 16>;

        MatrixState CaptureMatrix(const Matrix4f &matrix) {
            MatrixState state;
            std::copy_n(matrix.m, state.size(), state.begin());
            return state;
        }

        struct BatchState {
            const MeshVertexBuffer *buffer;
            std::vector<GLVoxelisationShader::Surface> surfaces;
            std::vector<MatrixState> transforms;

            bool operator==(const BatchState &other) const {
                return buffer == other.buffer && surfaces == other.surfaces && transforms == other.transforms;
            }
        };

        using SceneState = std::unordered_map<Pipeline *, std::unordered_map<Material *, BatchState> >;

        enum class CaptureMode { DebugColour, Radiance, ShadowCasters };

        SceneState CaptureBatches(const ModelBatches &batches, CaptureMode mode, bool captureBounceSurfaces = false) {
            SceneState state;
            for (const auto &[pipeline, materials]: batches) {
                auto *vertices = static_cast<GLGraphicsPipeline *>(pipeline)->getVertexBuffer();
                if (vertices->getType() != VertexBufferType::MESH) continue;
                auto *mesh = static_cast<MeshVertexBuffer *>(vertices);
                auto &batchStates = state[pipeline];
                for (const auto &[material, objects]: materials) {
                    BatchState batch{mesh, {}, {}};
                    for (const auto *object: objects) {
                        batch.transforms.push_back(
                            CaptureMatrix(object->getParent()->getWorldTransform().getTransformationMatrix()));
                        if (mode == CaptureMode::ShadowCasters) continue;
                        for (uint32_t i = 0; i < mesh->getSubmeshCount(); ++i) {
                            auto surface = GLVoxelisationShader::GetSurface(
                                object->ResolveSubmeshMaterial(mesh->getModel(), i, material), captureBounceSurfaces);
                            if (mode == CaptureMode::DebugColour) surface.emission = 0.0f;
                            batch.surfaces.push_back(surface);
                        }
                    }
                    if (mode == CaptureMode::ShadowCasters) std::sort(batch.transforms.begin(), batch.transforms.end());
                    batchStates.emplace(material, std::move(batch));
                }
            }
            return state;
        }

        using BakeClock = std::chrono::steady_clock;
        constexpr int VoxelShadowSize = 1024;
        constexpr float CubeDiagonal = 1.7320508f;

        class VoxelShadowShader final : public GLShader {
        public:
            void UpdateObjectUniforms(GameObject *object, uint32_t) const override {
                SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix());
            }
        };

        struct LightingState {
            const Light *source = nullptr;
            Vector3f direction = Vector3f(0, 1, 0);
            Vector3f radiance = Vector3f(0);

            bool operator==(const LightingState &other) const {
                return source == other.source && direction == other.direction && radiance == other.radiance;
            }
        };

        LightingState CaptureSun(const Renderer &renderer) {
            const auto *sun = ShadowCascades::FindSun(renderer);
            if (sun) {
                const Vector3f radiance = sun->getColour() * sun->getIntensity();
                if (radiance.x > 0 || radiance.y > 0 || radiance.z > 0)
                    return {sun, sun->getParent()->getWorldTransform().getPosition().normalise(), radiance};
            }
            return {};
        }

        struct BakeInputs {
            Vector3f center;
            float extent;
            int traceDepth;
            bool voxelSunShadows;
            bool directionalMips;
            uint32_t environment;
            SceneState meshes;
            SceneState transparentCasters;
            std::vector<TerrainVoxelInput> terrains;

            bool operator==(const BakeInputs &other) const {
                return center == other.center && extent == other.extent && traceDepth == other.traceDepth
                       && voxelSunShadows == other.voxelSunShadows && directionalMips == other.directionalMips
                       && environment == other.environment && meshes == other.meshes && transparentCasters == other.
                       transparentCasters && terrains == other.terrains;
            }
        };
    }

    namespace {
        struct GpuTimer {
            GLuint query = 0;
            bool pending = false;
            double milliseconds = -1.0;

            ~GpuTimer() {
                if (query) glDeleteQueries(1, &query);
            }

            void Collect() {
                if (!pending) return;
                GLint available = GL_FALSE;
                glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
                if (available != GL_TRUE) return;
                GLuint64 nanoseconds = 0;
                glGetQueryObjectui64v(query, GL_QUERY_RESULT, &nanoseconds);
                milliseconds = static_cast<double>(nanoseconds) / 1.0e6;
                pending = false;
            }
        };

        struct ScopedGpuTimer {
            explicit ScopedGpuTimer(GpuTimer &timer) : timer(timer), active(!timer.pending) {
                if (!active) return;
                if (!timer.query) glCreateQueries(GL_TIME_ELAPSED, 1, &timer.query);
                glBeginQuery(GL_TIME_ELAPSED, timer.query);
            }

            ~ScopedGpuTimer() { Stop(); }

            void Stop() {
                if (!active) return;
                glEndQuery(GL_TIME_ELAPSED);
                timer.pending = true;
                active = false;
            }

            GpuTimer &timer;
            bool active;
        };
    }

    struct GLVoxelRendererPass::BakeCache {
        std::optional<BakeInputs> debug;
        std::optional<BakeInputs> radiance;
        LightingState lighting;
        BakeClock::time_point completed;
        GpuTimer diffuseCompositeTiming;
        GpuTimer timings[static_cast<size_t>(GpuStage::Count)];
        GLuint directionalVolumes[6] = {};
        std::unique_ptr<GLShader> directionalMipShader;

        GpuTimer &Timer(GpuStage stage) { return timings[static_cast<size_t>(stage)]; }
    };

    double GLVoxelRendererPass::GetGpuMilliseconds(GpuStage stage) const {
        const double milliseconds = cache->Timer(stage).milliseconds;
        if (stage == GpuStage::Composite && milliseconds >= 0.0 && renderer->screenSpaceReflections &&
            settings.globalIllumination && !settings.debugView)
            return milliseconds + std::max(0.0, cache->diffuseCompositeTiming.milliseconds);
        return milliseconds;
    }

    GLVoxelRendererPass::GLVoxelRendererPass(Renderer *renderer)
        : RenderStage(renderer), cache(std::make_unique<BakeCache>()) {
    }

    GLVoxelRendererPass::~GLVoxelRendererPass() {
        if (cascadeBuffer) glDeleteBuffers(1, &cascadeBuffer);
        if (terrainParameters) glDeleteBuffers(1, &terrainParameters);
        glDeleteTextures(1, &screenColour);
        glDeleteTextures(1, &screenConfidence);
        if (voxelisationShader.pointer)
            manager->getAssetManager()->removeReference<Shader>(voxelisationShader.handle);
        if (framebuffer) glDeleteFramebuffers(1, &framebuffer);
        if (shadowFramebuffer) glDeleteFramebuffers(1, &shadowFramebuffer);
        if (shadowDepth) glDeleteTextures(1, &shadowDepth);
        if (indirectImage) glDeleteTextures(1, &indirectImage);
        if (debugVolume) glDeleteTextures(1, &debugVolume);
        if (emissionVolume) glDeleteTextures(1, &emissionVolume);
        if (radianceVolume) glDeleteTextures(1, &radianceVolume);
        ReleaseDirectionalMips();
        ReleaseBounceVolumes();
        glDeleteTextures(1, &bounceSurfaces);
        glDeleteTextures(1, &scrollScratch);
    }

    bool GLVoxelRendererPass::Initialise() {
        if (initialisationAttempted) return ready;
        initialisationAttempted = true;

        AssetManager *assets = manager->getAssetManager();
        auto shader = assets->loadShader(ShaderName::Voxelisation);
        if (!shader) return false;
        voxelisationShader = *shader;
        assets->addReference<Shader>(voxelisationShader.handle);

        auto loadCompute = [](const char *path) -> std::unique_ptr<GLShader> {
            auto program = std::make_unique<GLShader>();
            if (!program->AddShader(ResourceLoader::LoadShaderGL(path), COMPUTE_SHADER) || !program->CompileShader())
                return nullptr;
            return program;
        };

        injectShader = loadCompute("opengl/voxel/voxel_inject_CS.glsl");
        scrollShader = loadCompute("opengl/voxel/voxel_scroll_CS.glsl");
        mipShader = loadCompute("opengl/voxel/voxel_mip_CS.glsl");
        cache->directionalMipShader = loadCompute("opengl/voxel/voxel_directional_mip_CS.glsl");
        compositeShader = loadCompute("opengl/voxel/voxel_view_CS.glsl");
        bounceShader = loadCompute("opengl/voxel/voxel_bounce_CS.glsl");
        occluderMipShader = loadCompute("opengl/voxel/voxel_occluder_mip_CS.glsl");
        filterShader = loadCompute("opengl/voxel/voxel_filter_CS.glsl");
        if (!injectShader || !scrollShader || !mipShader || !cache->directionalMipShader || !compositeShader ||
            !bounceShader || !occluderMipShader || !filterShader)
            return false;
        scrollShader->AddUniform("scrollDelta");
        for (const char *uniform: {
                 "sunDirection", "sunRadiance", "lightViewProjection", "shadowBias",
                 "useVoxelSunShadows", "useTerrainOcclusion", "voxelOccluders", "voxelOcclusionMin",
                 "voxelOcclusionExtent"
             })
            injectShader->AddUniform(uniform);
        cache->directionalMipShader->AddUniform("directionIndex");
        filterShader->AddUniform("useScreenReflections");
        filterShader->AddUniform("applyDiffuse");
        filterShader->AddUniform("applySpecular");
        filterShader->AddUniform("giStrength");
        filterShader->AddUniform("terrainReceivesGI");
        filterShader->AddUniform("voxelSize");
        filterShader->AddUniform("traceStride");
        filterShader->AddUniform("volumeMin");
        filterShader->AddUniform("volumeExtent");
        compositeShader->AddUniform("debugCascadeColour");
        compositeShader->AddUniform("traceStride");
        compositeShader->AddUniform("terrainReceivesGI");
        bounceShader->AddUniform("voxelOcclusionMin");
        bounceShader->AddUniform("voxelOcclusionExtent");
        injectShader->AddUniform("globalInjection");
        bounceShader->AddUniform("writeNext");
        bounceShader->AddUniform("useEnvironment");
        filterShader->AddUniform("cameraPosition");
        filterShader->AddUniform("maxReflectionLOD");

        shadowShader = std::make_unique<VoxelShadowShader>();
        if (!shadowShader->AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/voxel_shadow_VS.glsl"), VERTEX_SHADER)
            || !shadowShader->AddShader(ResourceLoader::LoadShaderGL("opengl/shadow/depth_pass_FS.glsl"),
                                        FRAGMENT_SHADER)
            || !shadowShader->CompileShader())
            return false;
        shadowShader->AddUniform("m_transform");
        shadowShader->AddUniform("lightViewProjection");

        for (const char *uniform: {"debugView", "volumeMin", "volumeExtent", "inverseViewProjection"})
            compositeShader->AddUniform(uniform);

        glCreateFramebuffers(1, &framebuffer);
        glNamedFramebufferDrawBuffer(framebuffer, GL_NONE);
        glNamedFramebufferReadBuffer(framebuffer, GL_NONE);
        ready = true;
        return true;
    }

    void GLVoxelRendererPass::SetResolution(int size) {
        if (size == resolution) return;
        resolution = size;
        glDeleteTextures(1, &debugVolume);
        glDeleteTextures(1, &emissionVolume);
        glDeleteTextures(1, &radianceVolume);
        ReleaseDirectionalMips();
        ReleaseBounceVolumes();
        glDeleteTextures(1, &bounceSurfaces);
        glDeleteTextures(1, &scrollScratch);
        bounceSurfaces = scrollScratch = 0;
        debugVolume = emissionVolume = radianceVolume = 0;
        cache->debug.reset();
        cache->radiance.reset();

        glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, size);
        glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, size);
    }

    GLuint GLVoxelRendererPass::GetPackedVolume(GLuint &texture, bool mipmaps) {
        if (!texture) {
            glCreateTextures(GL_TEXTURE_3D, 1, &texture);
            int levels = 1;
            if (mipmaps) {
                for (int size = resolution; size > 1; size /= 2) ++levels;
            }
            glTextureStorage3D(texture, levels, GL_R32UI, resolution, resolution, resolution);
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        return texture;
    }

    void GLVoxelRendererPass::AllocateRadianceVolume(GLuint &texture, int size) {
        if (texture) return;
        int levels = 1;
        for (int width = size; width > 1; width /= 2) ++levels;
        glCreateTextures(GL_TEXTURE_3D, 1, &texture);
        glTextureStorage3D(texture, levels, GL_RGBA16F, size, size, size);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    void GLVoxelRendererPass::GenerateOccluderMips() const {
        PR_PROFILE("Build voxel occlusion hierarchy");
        occluderMipShader->Bind(nullptr);
        int size = resolution;
        for (int level = 1; size > 1; ++level) {
            size /= 2;
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glBindImageTexture(0, debugVolume, level - 1, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
            glBindImageTexture(1, debugVolume, level, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
            glDispatchCompute((size + 3) / 4, (size + 3) / 4, (size + 3) / 4);
        }
        occluderMipShader->Unbind();
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    }

    void GLVoxelRendererPass::GenerateRadianceMips(GLuint texture) {
        PR_PROFILE("Generate voxel radiance mips");
        if (settings.directionalMips) {
            auto &shader = *cache->directionalMipShader;
            shader.Bind(nullptr);
            for (int direction = 0; direction < 6; ++direction) {
                GLuint &volume = cache->directionalVolumes[direction];
                AllocateRadianceVolume(volume, resolution / 2);
                shader.SetUniformi("directionIndex", direction);
                int size = resolution / 2;
                for (int level = 0; size > 0; ++level, size /= 2) {
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                    glBindImageTexture(0, level == 0 ? texture : volume, std::max(0, level - 1),
                                       GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
                    glBindImageTexture(1, volume, level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
                    glDispatchCompute((size + 3) / 4, (size + 3) / 4, (size + 3) / 4);
                }
            }
            shader.Unbind();
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            return;
        }

        mipShader->Bind(nullptr);
        int size = resolution;
        for (int level = 1; size > 1; ++level) {
            size /= 2;
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glBindImageTexture(0, texture, level - 1, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
            glBindImageTexture(1, texture, level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            glDispatchCompute((size + 3) / 4, (size + 3) / 4, (size + 3) / 4);
        }
        mipShader->Unbind();
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    void GLVoxelRendererPass::ReleaseDirectionalMips() {
        if (!cache->directionalVolumes[0]) return;
        glBindTextures(9, 6, nullptr);
        glDeleteTextures(6, cache->directionalVolumes);
        for (auto &volume: cache->directionalVolumes) volume = 0;
        cache->radiance.reset();
    }

    void GLVoxelRendererPass::ReleaseBounceVolumes() {
        if (bounceVolumes[0]) glDeleteTextures(2, bounceVolumes);
        bounceVolumes[0] = bounceVolumes[1] = 0;
    }

    Matrix4f GLVoxelRendererPass::RenderSunShadow(const Vector3f &direction) {
        PR_PROFILE("Bake voxel sun shadow");
        if (!shadowDepth) {
            glCreateTextures(GL_TEXTURE_2D, 1, &shadowDepth);
            glTextureStorage2D(shadowDepth, 1, GL_DEPTH_COMPONENT32F, VoxelShadowSize, VoxelShadowSize);
            glTextureParameteri(shadowDepth, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(shadowDepth, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(shadowDepth, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(shadowDepth, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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

    bool GLVoxelRendererPass::CanUseResolution(int index) const {
        if (!IsSupported() || index < 0 || index >= static_cast<int>(std::size(Resolutions))) return false;
        constexpr uint64_t MiB = 1024 * 1024;
        if (!voxelBudget) {
            glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxVolumeSize);
            GLint freeKiB = 0;
            if (GLAD_GL_NVX_gpu_memory_info)
                glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &freeKiB);
            else if (GLAD_GL_ATI_meminfo) {
                GLint freeMemory[4] = {};
                glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, freeMemory);
                freeKiB = freeMemory[0];
            }
            // Leave room for frame targets, drivers, and scene assets.
            voxelBudget = freeKiB > 0 ? static_cast<uint64_t>(freeKiB) * 1024 * 3 / 4 : 768 * MiB;
        }

        const uint64_t size = Resolutions[index];
        if (size > static_cast<uint64_t>(maxVolumeSize)) return false;
        const uint64_t cells = size * size * size;
        const uint64_t mipCells = cells * 8 / 7;
        uint64_t bytes = 4 * mipCells + 12 * cells; // Colour/occlusion hierarchy.
        if (settings.globalIllumination) {
            bytes += 8 * mipCells;
            if (settings.traceDepth > 1) bytes += 8 * mipCells;
            if (settings.traceDepth > 2) bytes += 8 * mipCells;
            if (settings.directionalMips) bytes += 6 * 8 * (mipCells - cells);
        }
        VoxelCascadeSnapshot snapshot;
        const auto snapshotBytes = MakeVoxelCascades(snapshot, unsigned(size), renderer->voxelCascadeCount,
                                                     {0, 0, 0}, settings.extent, settings.directionalMips);
        // Conservatively budget each far grid with all bounce and directional chains.
        bytes += snapshotBytes + (snapshot.meta[0] - 1) * 64ull * 64 * 64 * 64;
        GLint64 maxBlock = 0;
        glGetInteger64v(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxBlock);
        return snapshotBytes <= size_t(maxBlock) && bytes <= voxelBudget;
    }

    void GLVoxelRendererPass::BindOccluders(const GLShader &shader, uint32_t textureUnit) const {
        if (IsSupported()) glBindTextureUnit(textureUnit, usesOccluders ? debugVolume : 0);
        shader.SetUniformi("voxelOccluders", textureUnit);
        shader.SetUniform("voxelOcclusionMin", settings.center - Vector3f(settings.extent * 0.5f));
        shader.SetUniformf("voxelOcclusionExtent", settings.extent);
    }

    void GLVoxelRendererPass::BindWaterLighting(const GLShader &shader) const {
        const bool active = ready && usesOccluders && settings.globalIllumination && !settings.debugView;
        if (IsSupported()) {
            glBindTextureUnit(0, active ? debugVolume : 0);
            glBindTextureUnit(1, active ? radianceVolume : 0);
        }
        shader.SetUniform("waterVoxelBounds",
                          Vector4f(settings.center - Vector3f(settings.extent * 0.5f), settings.extent));
        shader.SetUniform("waterVoxelOptions",
                          Vector4f(active ? float(resolution) : 0, settings.giStrength, float(active), 0));
    }

    void GLVoxelRendererPass::Render() {
        usesOccluders = false;
        for (auto &timer: cache->timings) timer.Collect();
        cache->diffuseCompositeTiming.Collect();
        if (!settings.directionalMips) ReleaseDirectionalMips();
        if (!IsSupported() || (!settings.debugView && !settings.globalIllumination && !settings.voxelSunShadows)
            || !Initialise())
            return;

        settings.resolutionIndex = std::clamp(settings.resolutionIndex, 0,
                                              static_cast<int>(std::size(Resolutions)) - 1);
        while (settings.resolutionIndex > 0 && !CanUseResolution(settings.resolutionIndex))
            --settings.resolutionIndex;
        SetResolution(Resolutions[settings.resolutionIndex]);
        renderer->UpdateVoxelFollow(unsigned(resolution));

        VoxelCascadeSnapshot snapshot;
        const auto center = renderer->voxelFollowCamera ? renderer->getCamera()->getPosition() : settings.center;
        const auto bytes = MakeVoxelCascades(snapshot, resolution, renderer->voxelCascadeCount,
                                             {center.x, center.y, center.z}, settings.extent, settings.directionalMips);
        const auto minimum = settings.center - Vector3f(settings.extent / 2);
        snapshot.bounds[0] = {minimum.x, minimum.y, minimum.z, settings.extent};
        bool relight = cascadeBufferBytes != bytes;
        for (unsigned i = 0; i < snapshot.meta[0]; ++i) {
            if (i && !farCascades[i - 1]) farCascades[i - 1] = std::make_unique < GLVoxelRendererPass > (renderer);
            auto &pass = i ? *farCascades[i - 1] : *this;
            if (i) {
                pass.settings = settings;
                const auto &b = snapshot.bounds[i];
                pass.settings.center = Vector3f(b[0], b[1], b[2]) + Vector3f(b[3] / 2);
                pass.settings.extent = b[3];
                pass.Initialise();
                pass.SetResolution(64);
                for (auto &timer: pass.cache->timings) timer.Collect();
            }
            renderer->voxelUpdate = {};
            pass.globalInjection = false;
            pass.deferBounces = true;
            pass.RenderVolume(true);
            pass.usesOccluders = true;
            if (!settings.debugView && settings.globalIllumination) pass.RenderVolume(false);
            relight |= pass.lightingChanged;
            renderer->voxelCascadeUpdates[i] = renderer->voxelUpdate;
        }
        for (unsigned i = snapshot.meta[0]; i < 4; ++i) farCascades[i - 1].reset();
        renderer->voxelUpdate = renderer->voxelCascadeUpdates[0];
        if (relight && settings.globalIllumination && !settings.debugView) {
            ExportCascades(snapshot, bytes);
            for (unsigned i = 0; i < snapshot.meta[0]; ++i) {
                auto &pass = i ? *farCascades[i - 1] : *this;
                pass.cache->radiance.reset();
                pass.globalInjection = true;
                pass.RenderVolume(false);
            }
            ExportCascades(snapshot, bytes);
            ScopedGpuTimer bounceTimer(cache->Timer(GpuStage::Propagation));
            for (int bounce = 1; bounce < settings.traceDepth; ++bounce) {
                const bool last = bounce + 1 == settings.traceDepth;
                for (unsigned i = 0; i < snapshot.meta[0]; ++i)
                    (i ? *farCascades[i - 1] : *this).CascadeBounce(bounce, last);
                ExportCascades(snapshot, bytes, !last);
            }
        } else if (!settings.globalIllumination || settings.debugView) ExportCascades(snapshot, bytes);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cascadeBuffer);
    }

    void GLVoxelRendererPass::CascadeBounce(int bounce, bool last) {
        if (!last) AllocateRadianceVolume(bounceVolumes[0], resolution);
        glBindImageTexture(0, bounceSurfaces, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
        glBindImageTexture(1, radianceVolume, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindImageTexture(2, last ? 0 : bounceVolumes[0], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        bounceShader->Bind(nullptr);
        bounceShader->SetUniform("voxelOcclusionMin", settings.center - Vector3f(settings.extent / 2));
        bounceShader->SetUniformf("voxelOcclusionExtent", settings.extent);
        static_cast<GLRenderer *>(renderer)->getEnvironmentLighting().getEnvironmentMap()->Bind(5);
        bounceShader->SetUniformi("useEnvironment", bounce == 1);
        bounceShader->SetUniformi("writeNext", !last);
        glDispatchCompute(resolution / 4, resolution / 4, resolution / 4);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        GenerateRadianceMips(last ? radianceVolume : bounceVolumes[0]);
        for (unsigned i = 0; i < 3; ++i) glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    }

    void GLVoxelRendererPass::ExportCascades(const VoxelCascadeSnapshot &snapshot, size_t bytes, bool bounceSource) {
        if (!cascadeBuffer) glCreateBuffers(1, &cascadeBuffer);
        if (cascadeBufferBytes != bytes) {
            glNamedBufferData(cascadeBuffer, bytes, nullptr, GL_DYNAMIC_COPY);
            cascadeBufferBytes = bytes;
        }
        glNamedBufferSubData(cascadeBuffer, 0, sizeof(snapshot), &snapshot);
        glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT | GL_PIXEL_BUFFER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, cascadeBuffer);
        for (unsigned c = 0; c < snapshot.meta[0]; ++c) {
            auto &pass = c ? *farCascades[c - 1] : *this;
            const auto &grid = snapshot.grids[c];
            auto copy = [&](GLuint texture, unsigned word, bool integer, unsigned width) {
                for (unsigned level = 0; width; ++level, width >>= 1) {
                    const size_t size = size_t(width) * width * width * (integer ? 4 : 8);
                    glGetTextureImage(texture, level, integer ? GL_RED_INTEGER : GL_RGBA,
                                      integer ? GL_UNSIGNED_INT : GL_HALF_FLOAT, GLsizei(size),
                                      reinterpret_cast<void *>(size_t(word) * 4));
                    word += unsigned(size / 4);
                }
            };
            copy(pass.debugVolume, grid[1], true, grid[0]);
            if (pass.radianceVolume && settings.globalIllumination && !settings.debugView) {
                copy(bounceSource ? pass.bounceVolumes[0] : pass.radianceVolume, grid[2], false, grid[0]);
                if (settings.directionalMips) {
                    const unsigned chain = VoxelPyramid(grid[0]) - grid[0] * grid[0] * grid[0];
                    for (unsigned face = 0; face < 6; ++face)
                        copy(pass.cache->directionalVolumes[face], grid[3] + 2 * face * chain, false, grid[0] / 2);
                }
            }
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cascadeBuffer);
        renderer->voxelCascadeBytes = bytes;
        for (unsigned i = 0; i < snapshot.meta[0]; ++i) {
            // Conservative storage estimate, including bounce chains and scratch.
            const uint64_t n = snapshot.grids[i][0];
            renderer->voxelCascadeBytes += 64 * n * n * n;
        }
    }

    void GLVoxelRendererPass::RenderVolume(bool debugView) {
        if (!debugView) lightingChanged = false;
        const auto lighting = debugView ? LightingState{} : CaptureSun(*renderer);
        const int traceDepth = debugView ? 1 : std::clamp(settings.traceDepth, 1, MaxTraceDepth);
        BakeInputs inputs{
            settings.center, settings.extent, traceDepth,
            !debugView && UsesVoxelSunShadows(), !debugView && settings.directionalMips,
            !debugView ? static_cast<GLRenderer *>(renderer)->getEnvironmentLighting().getEnvironmentMap()->getID() : 0,
            CaptureBatches(renderer->getModels3D(), CaptureMode::Radiance, true), {}
        };
        const auto minimum = settings.center - Vector3f(settings.extent * 0.5f);
        inputs.terrains = CollectTerrainVoxels(*renderer, {minimum.x, minimum.y, minimum.z, settings.extent},
                                               unsigned(resolution), true);
        if (!debugView && lighting.source && !inputs.voxelSunShadows)
            inputs.transparentCasters = CaptureBatches(renderer->getModelsTransparency(), CaptureMode::ShadowCasters);
        auto &previous = debugView ? cache->debug : cache->radiance;
        if (previous && *previous == inputs) {
            if (debugView || lighting == cache->lighting) return;
            const auto interval = std::chrono::duration<float>(__EnvironmentMapConfig.atmosphereRefreshSeconds);
            if (lighting.source == cache->lighting.source && BakeClock::now() - cache->completed < interval) return;
        }
        if (!debugView && deferBounces && !globalInjection) {
            lightingChanged = true;
            return;
        }
        ScopedGpuTimer timer(cache->Timer(debugView ? GpuStage::Occluders : GpuStage::Injection));
        if (debugView) {
            VoxelScroll scroll;
            if (previous && inputs.terrains.empty()) {
                auto comparison = inputs;
                comparison.center = previous->center;
                if (comparison == *previous)
                    scroll = PlanVoxelScroll({
                                                 previous->center.x, previous->center.y, previous->center.z,
                                                 previous->extent
                                             },
                                             {settings.center.x, settings.center.y, settings.center.z, settings.extent},
                                             unsigned(resolution));
            }
            GetPackedVolume(debugVolume, true);
            GetPackedVolume(bounceSurfaces);
            GetPackedVolume(emissionVolume);
            const GLuint volumes[] = {debugVolume, bounceSurfaces, emissionVolume};
            const GLuint empty = 0;
            for (GLuint volume: volumes) {
                if (scroll.incremental) {
                    GetPackedVolume(scrollScratch);
                    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
                    glCopyImageSubData(volume, GL_TEXTURE_3D, 0, 0, 0, 0, scrollScratch, GL_TEXTURE_3D, 0, 0, 0, 0,
                                       resolution, resolution, resolution);
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                    glBindImageTexture(0, scrollScratch, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
                    glBindImageTexture(1, volume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
                    scrollShader->Bind(nullptr);
                    scrollShader->SetUniform("scrollDelta",
                                             Vector3f(float(scroll.delta[0]), float(scroll.delta[1]),
                                                      float(scroll.delta[2])));
                    glDispatchCompute((resolution + 3) / 4, (resolution + 3) / 4, (resolution + 3) / 4);
                    scrollShader->Unbind();
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
                } else glClearTexImage(volume, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &empty);
            }
            for (unsigned i = 0; i < 3; ++i) glBindImageTexture(i, volumes[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
            GLint viewport[4], drawFramebuffer, polygonMode[2];
            glGetIntegerv(GL_VIEWPORT, viewport);
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
            glGetIntegerv(GL_POLYGON_MODE, polygonMode);
            const bool culling = glIsEnabled(GL_CULL_FACE), scissor = glIsEnabled(GL_SCISSOR_TEST);
            const bool discard = glIsEnabled(GL_RASTERIZER_DISCARD);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);
            glDisable(GL_SCISSOR_TEST);
            glDisable(GL_RASTERIZER_DISCARD);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
            glViewport(0, 0, resolution, resolution);
            auto *shader = static_cast<GLVoxelisationShader *>(voxelisationShader.pointer);
            shader->Bind(nullptr);
            shader->SetUniform("volumeMin", minimum);
            shader->SetUniformf("volumeExtent", settings.extent);
            shader->SetUniformi("incremental", scroll.incremental);
            shader->SetUniformi("gridResolution", resolution);
            shader->SetUniform("scrollDelta",
                               Vector3f(float(scroll.delta[0]), float(scroll.delta[1]), float(scroll.delta[2])));
            for (const auto &[pipeline, batches]: renderer->getModels3D()) {
                auto *buffer = static_cast<GLGraphicsPipeline *>(pipeline)->getVertexBuffer();
                if (buffer->getType() != VertexBufferType::MESH) continue;
                auto &model = static_cast<MeshVertexBuffer *>(buffer)->getModel();
                buffer->Bind(nullptr);
                for (const auto &[material, objects]: batches)
                    for (uint32_t i = 0; i < buffer->getSubmeshCount(); ++i)
                        for (const auto *object: objects) {
                            shader->SetSurface(
                                GLVoxelisationShader::GetSurface(object->ResolveSubmeshMaterial(model, i, material),
                                                                 true));
                            shader->UpdateObjectUniforms(object->getParent());
                            buffer->Draw(nullptr, i);
                        }
                buffer->Unbind();
            }
            shader->Unbind();
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            glPolygonMode(GL_FRONT_AND_BACK, polygonMode[0]);
            if (culling) glEnable(GL_CULL_FACE);
            if (scissor) glEnable(GL_SCISSOR_TEST);
            if (discard) glEnable(GL_RASTERIZER_DISCARD);
            if (!inputs.terrains.empty()) {
                if (!terrainShader) {
                    terrainShader = std::make_unique<GLShader>();
                    terrainShader->AddShader(ResourceLoader::LoadShaderGL("opengl/voxel/terrain_CS.glsl"),
                                             COMPUTE_SHADER);
                    terrainShader->CompileShader();
                    glGenBuffers(1, &terrainParameters);
                }
                terrainShader->Bind(nullptr);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, terrainParameters);
                for (const auto &terrain: inputs.terrains) {
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, terrainParameters);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TerrainVoxelParameters), &terrain.parameters,
                                 GL_STREAM_DRAW);
                    for (unsigned i = 0; i < terrain.textures.size(); ++i) terrain.textures[i]->Bind(i);
                    const auto &region = terrain.parameters.region;
                    glDispatchCompute((unsigned(region[2]) + 7) / 8, (unsigned(region[3]) + 7) / 8, 1);
                }
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
                terrainShader->Unbind();
            }
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
            GenerateOccluderMips();
            renderer->voxelUpdate.mode = scroll.incremental
                                             ? Renderer::VoxelUpdate::Scrolled
                                             : Renderer::VoxelUpdate::Rebuilt;
            renderer->voxelUpdate.dirtyCells = scroll.incremental
                                                   ? scroll.dirtyCells
                                                   : uint64_t(resolution) * resolution * resolution;
            cache->radiance.reset(); // Changed occlusion may affect light throughout the retained region.
        } else {
            AllocateRadianceVolume(radianceVolume, resolution);
            GLint viewport[4], drawFramebuffer;
            glGetIntegerv(GL_VIEWPORT, viewport);
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
            const auto lightSpace = lighting.source && !inputs.voxelSunShadows
                                        ? RenderSunShadow(lighting.direction)
                                        : Matrix4f::Identity();
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            injectShader->Bind(nullptr);
            injectShader->SetUniformi("globalInjection", globalInjection);
            BindOccluders(*injectShader, 8);
            injectShader->SetUniform("sunDirection", lighting.direction);
            injectShader->SetUniform("sunRadiance", lighting.radiance);
            injectShader->SetUniform("lightViewProjection", lightSpace);
            injectShader->SetUniformf("shadowBias", 1.5f * CubeDiagonal / (VoxelShadowSize * (CubeDiagonal + 1.0f)));
            injectShader->SetUniformi("useVoxelSunShadows", inputs.voxelSunShadows);
            injectShader->SetUniformi("useTerrainOcclusion", !inputs.terrains.empty());
            glBindTextureUnit(1, shadowDepth);
            glBindImageTexture(0, debugVolume, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
            glBindImageTexture(1, bounceSurfaces, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
            glBindImageTexture(2, emissionVolume, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
            glBindImageTexture(3, radianceVolume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            glDispatchCompute((resolution + 3) / 4, (resolution + 3) / 4, (resolution + 3) / 4);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
            injectShader->Unbind();
            timer.Stop();
            if (traceDepth == 1) ReleaseBounceVolumes();
            {
                ScopedGpuTimer mipTimer(cache->Timer(GpuStage::RadianceMips));
                GenerateRadianceMips(radianceVolume);
            }
            cache->lighting = lighting;
            lightingChanged = true;
            cache->completed = BakeClock::now();
        }
        for (unsigned i = 0; i < 4; ++i) glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
        previous = std::move(inputs);
    }

    void GLVoxelRendererPass::AllocateIndirectImage(uint32_t width, uint32_t height) {
        if (indirectWidth == width && indirectHeight == height) return;
        indirectWidth = width;
        indirectHeight = height;
        if (indirectImage) glDeleteTextures(1, &indirectImage);
        glCreateTextures(GL_TEXTURE_2D, 1, &indirectImage);
        glTextureStorage2D(indirectImage, 1, GL_RGBA16F, width, height);
        glTextureParameteri(indirectImage, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(indirectImage, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    void GLVoxelRendererPass::RenderScreenReflections(Texture *output) {
        if (!IsSupported()) return;
        if (!screenShader) {
            auto shader = std::make_unique<GLShader>();
            if (!shader->AddShader(ResourceLoader::LoadShaderGL("opengl/postProcessing/screen_reflection_CS.glsl"),
                                   COMPUTE_SHADER)
                || !shader->CompileShader())
                return;
            for (auto name: {"m_view", "m_projection", "cameraPosition", "maxReflectionLOD"}) shader->AddUniform(name);
            screenShader = std::move(shader);
        }
        const auto width = output->getWidth(), height = output->getHeight();
        if (width != screenWidth || height != screenHeight) {
            glDeleteTextures(1, &screenColour);
            glDeleteTextures(1, &screenConfidence);
            glCreateTextures(GL_TEXTURE_2D, 1, &screenColour);
            glTextureStorage2D(screenColour, 1, GL_RGBA32F, width, height);
            glCreateTextures(GL_TEXTURE_2D, 1, &screenConfidence);
            glTextureStorage2D(screenConfidence, 1, GL_R16F, width, height);
            screenWidth = width;
            screenHeight = height;
        }
        const GLuint target = static_cast<GLTexture *>(output)->getTextureID();
        glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
        glCopyImageSubData(target, GL_TEXTURE_2D, 0, 0, 0, 0, screenColour, GL_TEXTURE_2D, 0, 0, 0, 0, width, height,
                           1);
        auto *gl = static_cast<GLRenderer *>(renderer);
        auto *main = gl->getMainPass();
        screenShader->Bind(nullptr);
        screenShader->SetUniform("m_view", renderer->getCamera()->getViewMatrix());
        screenShader->SetUniform("m_projection", renderer->getCamera()->getProjectionMatrix());
        screenShader->SetUniform("cameraPosition", renderer->getCamera()->getPosition());
        screenShader->SetUniformf("maxReflectionLOD", __EnvironmentMapConfig.prefilterLevels - 1.0f);
        glBindTextureUnit(0, screenColour);
        main->getPositionMetallicImage()->Bind(1);
        main->getAlbedoRoughnessImage()->Bind(2);
        main->getNormalImage()->Bind(3);
        main->getBloomImage()->Bind(4);
        gl->getEnvironmentLighting().getPrefilterMap()->Bind(5);
        gl->getEnvironmentLighting().getBRDFMap()->Bind(6);
        glBindImageTexture(0, target, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, screenConfidence, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
        glDispatchCompute((width + 7) / 8, (height + 7) / 8, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16F);
        screenShader->Unbind();
    }

    void GLVoxelRendererPass::Composite(Texture *output, CompositeLighting lighting) {
        PR_PROFILE("Voxel composite");
        const uint32_t width = output->getWidth();
        const uint32_t height = output->getHeight();
        const GLuint outputId = static_cast<GLTexture *>(output)->getTextureID();
        const uint32_t groupsX = (width + 7) / 8;
        const uint32_t groupsY = (height + 7) / 8;

        compositeShader->Bind(nullptr);
        compositeShader->SetUniformi("debugView", settings.debugView);
        compositeShader->SetUniformi("terrainReceivesGI", renderer->terrainReceivesGI);
        compositeShader->SetUniform("volumeMin", settings.center - Vector3f(settings.extent * 0.5f));
        compositeShader->SetUniformf("volumeExtent", settings.extent);
        if (settings.debugView) {
            const Camera *camera = renderer->getCamera();
            compositeShader->SetUniform("inverseViewProjection",
                                        (camera->getProjectionMatrix() * camera->getViewMatrix()).Invert());
            const int selected = std::clamp(renderer->voxelDebugCascade, 0, int(renderer->voxelCascadeCount) - 1);
            const auto &pass = selected ? *farCascades[selected - 1] : *this;
            compositeShader->SetUniform("volumeMin", pass.settings.center - Vector3f(pass.settings.extent / 2));
            compositeShader->SetUniformf("volumeExtent", pass.settings.extent);
            compositeShader->SetUniformi("debugCascadeColour", renderer->voxelCascadeColours ? selected + 1 : 0);
            glBindTextureUnit(4, pass.debugVolume);
            glBindImageTexture(0, outputId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(groupsX, groupsY, 1);
            glBindTextureUnit(4, 0);
        } else {
            const uint32_t stride = settings.halfResolution ? 2 : 1;
            AllocateIndirectImage((width + stride - 1) / stride, (height + stride - 1) / stride);
            compositeShader->SetUniformi("traceStride", stride);
            const auto *main = static_cast<GLRenderer *>(renderer)->getMainPass();
            glBindTextureUnit(0, radianceVolume);
            glBindTextureUnit(8, debugVolume);
            auto &environment = static_cast<GLRenderer *>(renderer)->getEnvironmentLighting();
            environment.getEnvironmentMap()->Bind(5);
            main->getPositionMetallicImage()->Bind(1);
            main->getBloomImage()->Bind(2);
            main->getNormalImage()->Bind(3);
            const bool applyDiffuse = lighting != CompositeLighting::SpecularOnly;
            const bool applySpecular = lighting != CompositeLighting::DiffuseOnly;
            if (applyDiffuse) {
                glBindImageTexture(1, indirectImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
                ScopedGpuTimer timer(cache->Timer(GpuStage::Trace));
                glDispatchCompute((indirectWidth + 7) / 8, (indirectHeight + 7) / 8, 1);
                glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            }

            filterShader->Bind(nullptr);
            filterShader->SetUniformi("applyDiffuse", applyDiffuse);
            filterShader->SetUniformi("applySpecular", applySpecular);
            main->getAlbedoRoughnessImage()->Bind(2);
            filterShader->SetUniformi("terrainReceivesGI", renderer->terrainReceivesGI);
            filterShader->SetUniformi("useScreenReflections", applySpecular && renderer->screenSpaceReflections && screenConfidence);
            glBindImageTexture(1, screenConfidence, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16F);
            filterShader->SetUniform("cameraPosition", renderer->getCamera()->getPosition());
            main->getBloomImage()->Bind(4);
            environment.getIrradianceMap()->Bind(5);
            glBindTextureUnit(6, radianceVolume);
            environment.getPrefilterMap()->Bind(7);
            environment.getBRDFMap()->Bind(15);
            filterShader->SetUniformf("maxReflectionLOD", __EnvironmentMapConfig.prefilterLevels - 1.0f);
            filterShader->SetUniformf("giStrength", settings.giStrength);
            filterShader->SetUniformf("voxelSize", settings.extent / resolution);
            filterShader->SetUniformi("traceStride", stride);
            filterShader->SetUniform("volumeMin", settings.center - Vector3f(settings.extent * 0.5f));
            filterShader->SetUniformf("volumeExtent", settings.extent);
            glBindTextureUnit(0, indirectImage);
            glBindImageTexture(0, outputId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
            {
                ScopedGpuTimer timer(applySpecular ? cache->Timer(GpuStage::Composite)
                                                   : cache->diffuseCompositeTiming);
                glDispatchCompute(groupsX, groupsY, 1);
            }
            glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16F);
            glBindTextureUnit(0, 0);
            glBindTextureUnit(8, 0);
            if (settings.directionalMips) glBindTextures(9, 6, nullptr);
        }

        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT
                        | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        compositeShader->Unbind();
    }
}