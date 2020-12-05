#include "Includes.hpp"
#include "GLGraphicsPipeline.h"

namespace Prehistoric
{
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
		if (buff->getFrontFace() == FrontFace::CLOCKWISE)
		{
			glFrontFace(GL_CW);
		}
		else if (buff->getFrontFace() == FrontFace::COUNTER_CLOCKWISE)
		{
			glFrontFace(GL_CCW);
		}

		if (backfaceCulling && (buff->getFrontFace() != FrontFace::DOUBLE_SIDED))
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		buff->Draw(nullptr);
	}

	void GLGraphicsPipeline::UnbindPipeline() const
	{
		getVertexBuffer()->Unbind();

		GLPipeline::UnbindPipeline();
	}
};
