#include "Includes.hpp"
#include "GLShadowDepthPass.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"

#include "platform/opengl/rendering/shaders/forwardPlus/GLDepthPassShader.h"
#include "platform/opengl/buffer/GLUniformBufferObject.h"

#include "prehistoric/application/Application.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Prehistoric {
    GLShadowDepthPass::GLShadowDepthPass(Renderer *renderer)
        : RenderStage(renderer), framebuffer{nullptr} {
        matrices = std::make_unique<GLUniformBufferObject>(window, nullptr, 16 * 16 * sizeof(float));

        AssetManager *man = manager->getAssetManager();
        depthTexture = man->storeTexture(GLTexture::Storage2DArray(ShadowCascades::Resolution,
                                                                   ShadowCascades::Resolution,
                                                                   (int) ShadowCascades::Count, D32_LINEAR, Bilinear,
                                                                   ClampToEdge, false));
        man->addReference<Texture>(depthTexture.handle);

        depthShader = man->loadShader(ShaderName::ShadowDepthPass).value();
        man->addReference<Shader>(depthShader.handle);

        framebuffer = std::make_unique<GLFramebuffer>(window);
        framebuffer->Bind();
        framebuffer->addColourAttachment2D(depthTexture.pointer);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        framebuffer->Check();
        framebuffer->Unbind();
    }

    GLShadowDepthPass::~GLShadowDepthPass() {
        AssetManager *man = manager->getAssetManager();
        man->removeReference<Shader>(depthShader.handle);
        man->removeReference<Texture>(depthTexture.handle);
    }

    void GLShadowDepthPass::BindMatrices() const {
        matrices->BindBase(nullptr, 0);
    }

    void GLShadowDepthPass::Render() {
        PR_PROFILE("Depth pass");
        GLRenderer *rend = (GLRenderer *) renderer;

        uint32_t width = window->getWidth();
        uint32_t height = window->getHeight();

        sun = ShadowCascades::FindSun(*renderer);

        if (sun == nullptr) {
            rendered = false;
            return;
        }

        //update stuff
        Vector3f sunPos = sun->getParent()->getWorldTransform().getPosition();
        shadowView = renderer->getCamera()->getViewMatrix();
        cascades.Update(shadowView, renderer->getCamera()->getFov(), float(width) / height, sunPos.normalise(),
                        ShadowCascades::Resolution);


        matrices->Bind(0);
        matrices->MapBuffer();
        uint8_t *ptr = (uint8_t *) matrices->getMappedData();
        for (const auto &elem: cascades.matrices) {
            memcpy(ptr, elem.m, 16 * sizeof(float));
            ptr += (16 * sizeof(float));
        }
        matrices->UnmapBuffer();
        matrices->Unbind();

        framebuffer->Bind();
        framebuffer->Clear(0.0f);

        window->getSwapchain()->SetWindowSize(ShadowCascades::Resolution, ShadowCascades::Resolution);

        depthShader->Bind(nullptr);
        dynamic_cast<GLShadowDepthPassShader *>(depthShader.pointer)->UpdateGlobalUniforms(matrices.get());

        // Use mesh geometry for both opaque and transparent casters. The depth
        // shader currently treats transparent materials as solid silhouettes.
        const bool cullingEnabled = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);
        GLDepthPass::DrawBatches(rend->getModels3D(), *depthShader.pointer);
        GLDepthPass::DrawBatches(rend->getModelsTransparency(), *depthShader.pointer);
        if (cullingEnabled) glEnable(GL_CULL_FACE);

        window->getSwapchain()->SetWindowSize(width, height);

        rendered = true;
        depthShader->Unbind();
        framebuffer->Unbind();
    }
}