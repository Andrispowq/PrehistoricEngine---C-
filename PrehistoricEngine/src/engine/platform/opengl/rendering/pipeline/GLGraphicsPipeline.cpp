#include "Includes.hpp"
#include "prehistoric/application/Application.h"
#include "GLGraphicsPipeline.h"

namespace Prehistoric
{
#define CW 0
#define CCW 1
#define NO_CULL 0
#define CULL 1

	static char state_frontface = CCW;
	static char state_culling = CULL;

	GLGraphicsPipeline::GLGraphicsPipeline(Window* window, AssetManager* manager, ShaderHandle shader, VertexBufferHandle vbo)
		: GLPipeline(window, manager, shader), GraphicsPipeline(manager, vbo)
	{
	}

	void GLGraphicsPipeline::BindPipeline(CommandBuffer* buffer) const
	{
		GLPipeline::BindPipeline(buffer);
		vbo->Bind(buffer);
	}

	void GLGraphicsPipeline::RenderPipeline() const
	{
		GLPipeline::RenderPipeline();

        const bool mirrored = __WaterConfig.stage == WaterRenderStage::Reflection;
        const char winding = ((vbo->getFrontFace() == FrontFace::CLOCKWISE) != mirrored) ? CW : CCW;
        if (winding != state_frontface)
        {
            glFrontFace(winding == CW ? GL_CW : GL_CCW);
            state_frontface = winding;
        }

		if (backfaceCulling && (vbo->getFrontFace() != FrontFace::DOUBLE_SIDED) && (state_culling != CULL))
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			state_culling = CULL;
		}
		else if((!backfaceCulling || (vbo->getFrontFace() == FrontFace::DOUBLE_SIDED)) && (state_culling == CULL))
		{
			glDisable(GL_CULL_FACE);
			state_culling = NO_CULL;
		}

		for (uint32_t i = 0; i < vbo->getSubmeshCount(); i++)
		{
			if (vbo->getType() == VertexBufferType::PATCH)
			{
				vbo->Draw(buffer, i);
				continue;
			}

			MeshVertexBuffer* mesh_vbo = reinterpret_cast<MeshVertexBuffer*>(vbo.pointer);
			Model& model = mesh_vbo->getModel();
			int matIdx = model.getMesh(i).getMaterialIndex();

			if (matIdx != -1)
			{
				shader->UpdateMaterialUniforms(model.getMaterials()[matIdx], 0);
			}

			vbo->Draw(buffer, i);
		}
	}

	void GLGraphicsPipeline::UnbindPipeline() const
	{
		vbo->Unbind();
		GLPipeline::UnbindPipeline();
	}

	uint64_t GLGraphicsPipeline::GetHash()
	{
		if (hash)
			return hash;

		SetHashInternal(APIHashFlags::GL, PipelineTypeHashFlags::Graphics, shader.handle, vbo.handle);
		return hash;
	}
};
