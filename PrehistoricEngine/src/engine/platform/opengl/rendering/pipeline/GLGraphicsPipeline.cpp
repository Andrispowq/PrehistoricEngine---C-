#include "Includes.hpp"
#include "prehistoric/application/Application.h"
#include "GLGraphicsPipeline.h"
#include "platform/opengl/rendering/shaders/pbr/GLPBRShader.h"

namespace Prehistoric {
#define CW 0
#define CCW 1
#define NO_CULL 0
#define CULL 1

    static char state_frontface = CCW;
    static char state_culling = CULL;

    GLGraphicsPipeline::GLGraphicsPipeline(Window *window, AssetManager *manager, ShaderHandle shader,
                                           VertexBufferHandle vbo, std::optional<FrontFace> face)
        : GLPipeline(window, manager, shader), GraphicsPipeline(manager, vbo, face) {
    }

    void GLGraphicsPipeline::BindPipeline(CommandBuffer *buffer) const {
        GLPipeline::BindPipeline(buffer);
        vbo->Bind(buffer);
    }

    void GLGraphicsPipeline::RenderPipeline() const {
        DrawMeshes(nullptr);
    }

    void GLGraphicsPipeline::RenderObject(const RendererComponent &object) const {
        DrawMeshes(&object);
    }

    void GLGraphicsPipeline::DrawMeshes(const RendererComponent *object) const {
        GLPipeline::RenderPipeline();

        const bool mirrored = __WaterConfig.stage == WaterRenderStage::Reflection;
        const char winding = ((getFrontFace() == FrontFace::CLOCKWISE) != mirrored) ? CW : CCW;
        if (winding != state_frontface) {
            glFrontFace(winding == CW ? GL_CW : GL_CCW);
            state_frontface = winding;
        }

        if (backfaceCulling && (getFrontFace() != FrontFace::DOUBLE_SIDED) && (state_culling != CULL)) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            state_culling = CULL;
        } else if ((!backfaceCulling || (getFrontFace() == FrontFace::DOUBLE_SIDED)) && (state_culling == CULL)) {
            glDisable(GL_CULL_FACE);
            state_culling = NO_CULL;
        }

        auto *pbr = dynamic_cast<GLPBRShader *>(getShader());
        auto *renderer = pbr ? Application::Get().getEngineLayer()->getRenderingEngine()->getRenderer() : nullptr;
        for (uint32_t i = 0; i < vbo->getSubmeshCount(); i++) {
            if (vbo->getType() == VertexBufferType::PATCH) {
                vbo->Draw(buffer, i);
                continue;
            }

            MeshVertexBuffer *mesh_vbo = reinterpret_cast<MeshVertexBuffer *>(vbo.pointer);
            Model &model = mesh_vbo->getModel();
            int matIdx = model.getMesh(i).getMaterialIndex();

            if (pbr)
                pbr->SetSelected(object && renderer->isSelected(object->getParent(), i)
                                 && __WaterConfig.stage == WaterRenderStage::Default);
            if (object) {
                shader->UpdateMaterialUniforms(object->ResolveSubmeshMaterial(model, i, object->getMaterial()), 0);
            } else if (matIdx != -1) {
                shader->UpdateMaterialUniforms(model.getMaterials()[matIdx], 0);
            }

            vbo->Draw(buffer, i);
        }
    }

    void GLGraphicsPipeline::UnbindPipeline() const {
        vbo->Unbind();
        GLPipeline::UnbindPipeline();
    }

    uint64_t GLGraphicsPipeline::GetHash() {
        if (hash)
            return hash;

        SetHashInternal(APIHashFlags::GL, PipelineTypeHashFlags::Graphics, shader.handle, vbo.handle);
        return hash;
    }
};