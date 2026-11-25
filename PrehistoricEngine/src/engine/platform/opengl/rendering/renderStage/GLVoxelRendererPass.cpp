#include "Includes.hpp"
#include "GLVoxelRendererPass.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/rendering/shaders/voxel/GLVoxelisationShader.h"
#include "prehistoric/common/buffer/MeshVertexBuffer.h"
#include "prehistoric/core/node/component/renderer/RenderableComponent.h"

namespace Prehistoric
{
    GLVoxelRendererPass::GLVoxelRendererPass(Renderer* renderer)
        : RenderStage(renderer)
    {
    }

    GLVoxelRendererPass::~GLVoxelRendererPass()
    {
        if (voxelisationShader.pointer)
            manager->getAssetManager()->removeReference<Shader>(voxelisationShader.handle);
        if (framebuffer) glDeleteFramebuffers(1, &framebuffer);
        if (packedVolume) glDeleteTextures(1, &packedVolume);
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

    void GLVoxelRendererPass::AllocateVolume(int size)
    {
        if (size == resolution) return;
        resolution = size;
        glDeleteTextures(1, &packedVolume);
        glDeleteTextures(1, &radianceVolume);
        glCreateTextures(GL_TEXTURE_3D, 1, &packedVolume);
        glTextureStorage3D(packedVolume, 1, GL_R32UI, size, size, size);
        glTextureParameteri(packedVolume, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(packedVolume, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        int levels = 1;
        for (int width = size; width > 1; width /= 2) ++levels;
        glCreateTextures(GL_TEXTURE_3D, 1, &radianceVolume);
        glTextureStorage3D(radianceVolume, levels, GL_RGBA16F, size, size, size);
        glTextureParameteri(radianceVolume, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(radianceVolume, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(radianceVolume, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(radianceVolume, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(radianceVolume, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, size);
        glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, size);
    }

    void GLVoxelRendererPass::Render()
    {
        if (!IsSupported() || (!settings.debugView && !settings.globalIllumination) || !Initialise()) return;
        PR_PROFILE("Voxelisation");
        AllocateVolume(Resolutions[settings.resolutionIndex]);

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
        shader->UpdateGlobalUniforms(renderer);
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

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindImageTexture(1, radianceVolume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        resolveShader->Bind(nullptr);
        glDispatchCompute(resolution / 4, resolution / 4, resolution / 4);
        resolveShader->Unbind();
        glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        if (settings.globalIllumination && !settings.debugView) glGenerateTextureMipmap(radianceVolume);
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
        glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    }

    void GLVoxelRendererPass::Composite(Texture* output) const
    {
        PR_PROFILE("Voxel composite");
        const auto* main = static_cast<GLRenderer*>(renderer)->getMainPass();
        glBindTextureUnit(0, radianceVolume);
        if (!settings.debugView)
        {
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
        glBindTextureUnit(0, 0);
        compositeShader->Unbind();
    }
}
