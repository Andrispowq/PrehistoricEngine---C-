#include "Includes.hpp"
#include "GLPipeline.h"

#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	GLPipeline::GLPipeline(Window* window, ResourceStorage* resourceStorage, ShaderHandle shader)
		: Pipeline(window, resourceStorage, shader)
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
