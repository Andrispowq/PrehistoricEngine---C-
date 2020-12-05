#ifndef GL_GRAPHICS_PIPELINE_H
#define GL_GRAPHICS_PIPELINE_H

#include "engine/prehistoric/common/rendering/pipeline/GraphicsPipeline.h"
#include "GLPipeline.h"

#include <glew.h>

namespace Prehistoric
{
	class GLGraphicsPipeline : public GLPipeline, public GraphicsPipeline
	{
	public:
		GLGraphicsPipeline(Window* window, AssetManager* manager, size_t shaderID, size_t vboID);
		virtual ~GLGraphicsPipeline() {}

		virtual void BindPipeline(CommandBuffer* buffer) const override;
		virtual void RenderPipeline() const override;
		virtual void UnbindPipeline() const override;
	};
};

#endif