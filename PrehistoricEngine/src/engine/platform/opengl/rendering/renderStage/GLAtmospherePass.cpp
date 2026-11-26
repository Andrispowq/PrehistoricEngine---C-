#include "Includes.hpp"
#include "GLAtmospherePass.h"
#include "prehistoric/core/modules/atmosphere/AtmosphereCapture.h"
#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/opengl/rendering/shaders/atmosphere/GLAtmosphereScatteringShader.h"
#include "platform/opengl/rendering/shaders/atmosphere/GLAtmosphereShader.h"
#include "platform/opengl/rendering/shaders/forwardPlus/GLDepthPassShader.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/core/modules/atmosphere/Atmosphere.h"

namespace Prehistoric {
    static Matrix4f SphereTransform(const Atmosphere &atmosphere) {
        return atmosphere.getWorldTransform().getTransformationMatrix() *
               Matrix4f::Scaling(Vector3f(__EngineConfig.rendererFarPlane / 2.0f));
    }

    GLAtmospherePass::GLAtmospherePass(AssembledAssetManager &manager)
        : manager(manager) {
    }

    GLAtmospherePass::~GLAtmospherePass() {
        if (simplePipeline.pointer) manager.removeReference<Pipeline>(simplePipeline.handle);
        if (scatteringPipeline.pointer) manager.removeReference<Pipeline>(scatteringPipeline.handle);
    }

    Pipeline *GLAtmospherePass::GetPipeline() {
        PipelineHandle &pipeline = scatteringPipeline;
        if (!pipeline.pointer) {
            AssetManager *assets = manager.getAssetManager();
            auto sphere = assets->loadVertexBuffer(std::nullopt, "engine://models/sphere.obj").value();
            sphere->setFrontFace(FrontFace::DOUBLE_SIDED);
            auto shader = assets->loadShader(ShaderName::AtmosphereScattering).value();
            pipeline = manager.createPipeline(PipelineTypeHashFlags::Graphics, shader, sphere);
            manager.addReference<Pipeline>(pipeline.handle);
        }
        return pipeline.pointer;
    }

    void GLAtmospherePass::DrawScene(const Atmosphere &atmosphere, const Camera &camera) {
        const Matrix4f &view = camera.getOriginalViewMatrix();
        Draw(EnvironmentCaptureSource::FromScene(&atmosphere, __EngineConfig.rendererExposure),
             SphereTransform(atmosphere),
             view, camera.getProjectionMatrix(), false);
    }

    void GLAtmospherePass::DrawCapture(const EnvironmentCaptureSource &source, const Matrix4f &view,
                                       const Matrix4f &projection) {
        Draw(source, Matrix4f::Identity(), view, projection, true);
    }

    void GLAtmospherePass::Draw(const EnvironmentCaptureSource &source, const Matrix4f &transform, const Matrix4f &view,
                                const Matrix4f &projection, bool capture) {
        Pipeline *pipeline = GetPipeline();
        pipeline->BindPipeline(nullptr);
        auto *shader = static_cast<GLAtmosphereScatteringShader *>(pipeline->getShader());
        shader->UpdateUniforms(source, transform, view, projection, capture);
        pipeline->RenderPipeline();
        pipeline->UnbindPipeline();
    }

    void GLAtmospherePass::DrawDepth(const Atmosphere &atmosphere, const GLDepthPassShader &shader) {
        auto *sphere = static_cast<GLGraphicsPipeline *>(GetPipeline())->getVertexBuffer();
        shader.SetTransform(SphereTransform(atmosphere));
        sphere->Bind(nullptr);
        for (uint32_t i = 0; i < sphere->getSubmeshCount(); ++i) {
            sphere->Draw(nullptr, i);
        }
        sphere->Unbind();
    }
}