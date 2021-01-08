#ifndef GL_GRAPHICS_PIPELINE_H
#define GL_GRAPHICS_PIPELINE_H

#include "prehistoric/common/rendering/pipeline/GraphicsPipeline.h"
#include "GLPipeline.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLGraphicsPipeline : public GLPipeline, public GraphicsPipeline
	{
	public:
		GLGraphicsPipeline(Window* window, AssetManager* manager, ShaderHandle shader, VertexBufferHandle vbo);
		virtual ~GLGraphicsPipeline() {}

		virtual void BindPipeline(CommandBuffer* buffer) const override;
		virtual void RenderPipeline() const override;
		virtual void UnbindPipeline() const override;
	};
};

#endif