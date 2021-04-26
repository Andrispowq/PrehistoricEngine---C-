#include "Includes.hpp"
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

		if ((vbo->getFrontFace() == FrontFace::CLOCKWISE) && (state_frontface == CCW))
		{
			glFrontFace(GL_CW);
			state_frontface = CW;
		}
		else if ((vbo->getFrontFace() == FrontFace::COUNTER_CLOCKWISE) && (state_frontface == CW))
		{
			glFrontFace(GL_CCW);
			state_frontface = CCW;
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

		vbo->Draw(buffer);
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
