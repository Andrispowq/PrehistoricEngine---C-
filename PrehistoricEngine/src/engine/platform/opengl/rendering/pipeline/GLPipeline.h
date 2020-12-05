#ifndef GL_PIPELINE_H
#define GL_PIPELINE_H

#include "engine/prehistoric/common/rendering/pipeline/Pipeline.h"

#include <glew.h>

#include "engine/prehistoric/common/framework/Window.h"

namespace Prehistoric
{
	class GLPipeline : public Pipeline
	{
	public:
		GLPipeline(Window* window, AssetManager* manager, size_t shaderID);
		virtual ~GLPipeline() {}

		virtual void BindPipeline(CommandBuffer* buffer) const override;
		virtual void RenderPipeline() const override;
		virtual void UnbindPipeline() const override;

	private:
		mutable CommandBuffer* buffer; //The current commandbuffer
	};
};

#endif