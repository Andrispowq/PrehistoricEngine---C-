#ifndef GL_PIPELINE_H
#define GL_PIPELINE_H

#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include <glad/glad.h>

#include "prehistoric/common/framework/Window.h"

namespace Prehistoric
{
	class GLPipeline : public Pipeline
	{
	public:
		GLPipeline(Window* window, ResourceStorage* resourceStorage, ShaderHandle shader);
		virtual ~GLPipeline() {}

		virtual void BindPipeline(CommandBuffer* buffer) const override;
		virtual void RenderPipeline() const override;
		virtual void UnbindPipeline() const override;
	};
};

#endif