#include "Includes.hpp"
#include "GLPipeline.h"

namespace Prehistoric
{
	GLPipeline::GLPipeline(Window* window, AssetManager* manager, size_t shaderID)
		: Pipeline(window, manager, shaderID)
	{
	}

	void GLPipeline::BindPipeline(CommandBuffer* buffer) const
	{
		this->buffer = buffer;
		getShader()->Bind(buffer);
	}

	void GLPipeline::RenderPipeline() const
	{
	}

	void GLPipeline::UnbindPipeline() const
	{
		getShader()->Unbind();
	}
};
