#ifndef GL_SHADOW_DEPTH_PASS_H
#define GL_SHADOW_DEPTH_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"

#include "prehistoric/common/rendering/ShadowCascades.h"
#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/rendering/shaders/shadow/GLShadowDepthPassShader.h"

namespace Prehistoric {
    class AssembledAssetManager;
    class Renderer;
    class Light;


    class GLShadowDepthPass : public RenderStage {
    public:
        GLShadowDepthPass(Renderer *renderer);

        virtual ~GLShadowDepthPass();

        // Shadow resolution is independent of the window extent.
        void OnResized() override {
        }

        virtual void Render() override;

        const Light *getSun() const { return rendered ? sun : nullptr; }
        const Matrix4f &getShadowView() const { return shadowView; }
        const std::array<float, ShadowCascades::Count - 1> &getCascadeDistances() const { return cascades.splits; }

        void BindMatrices() const;

        bool hasShadowMap() const { return rendered; }
        TextureHandle getDepthTexture() const { return depthTexture; }

    private:
        std::unique_ptr<GLFramebuffer> framebuffer;
        ShaderHandle depthShader;
        TextureHandle depthTexture;

        ShadowCascades cascades;
        bool rendered = false;
        const Light *sun = nullptr;
        Matrix4f shadowView = Matrix4f::Identity();
        std::unique_ptr<UniformBufferObject> matrices;
    };
};

#endif