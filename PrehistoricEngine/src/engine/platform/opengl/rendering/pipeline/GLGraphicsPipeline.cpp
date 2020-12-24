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

	GLGraphicsPipeline::GLGraphicsPipeline(Window* window, AssetManager* manager, size_t shaderID, size_t vboID)
		: GLPipeline(window, manager, shaderID), GraphicsPipeline(manager, vboID)
	{
	}

	void GLGraphicsPipeline::BindPipeline(CommandBuffer* buffer) const
	{
		GLPipeline::BindPipeline(buffer);

		getVertexBuffer()->Bind(buffer);
	}

	void GLGraphicsPipeline::RenderPipeline() const
	{
		GLPipeline::RenderPipeline();

		VertexBuffer* buff = getVertexBuffer();
		if ((buff->getFrontFace() == FrontFace::CLOCKWISE) && (state_frontface == CCW))
		{
			glFrontFace(GL_CW);
			state_frontface = CW;
		}
		else if ((buff->getFrontFace() == FrontFace::COUNTER_CLOCKWISE) && (state_frontface == CW))
		{
			glFrontFace(GL_CCW);
			state_frontface = CCW;
		}

		if (backfaceCulling && (buff->getFrontFace() != FrontFace::DOUBLE_SIDED) && (state_culling != CULL))
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			state_culling = CULL;
		}
		else if((!backfaceCulling || (buff->getFrontFace() == FrontFace::DOUBLE_SIDED)) && (state_culling == CULL))
		{
			glDisable(GL_CULL_FACE);
			state_culling = NO_CULL;
		}

		buff->Draw(nullptr);
	}

	void GLGraphicsPipeline::UnbindPipeline() const
	{
		getVertexBuffer()->Unbind();

		GLPipeline::UnbindPipeline();
	}
};
