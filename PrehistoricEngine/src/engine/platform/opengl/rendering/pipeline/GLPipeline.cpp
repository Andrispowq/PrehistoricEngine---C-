#include "Includes.hpp"
#include "GLPipeline.h"

namespace Prehistoric
{
	GLPipeline::GLPipeline(Window* window, AssetManager* manager, ShaderHandle shader)
		: Pipeline(window, manager, shader)
	{
	}

	void GLPipeline::BindPipeline(CommandBuffer* buffer) const
	{
		this->buffer = buffer;
		shader->Bind(buffer);
	}

	void GLPipeline::RenderPipeline() const
	{
	}

	void GLPipeline::UnbindPipeline() const
	{
		shader->Unbind();
	}
};
