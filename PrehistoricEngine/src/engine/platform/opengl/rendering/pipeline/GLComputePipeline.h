#ifndef GL_COMPUTE_PIPELINE
#define GL_COMPUTE_PIPELINE

#include "prehistoric/common/rendering/pipeline/ComputePipeline.h"
#include "GLPipeline.h"

#include <glad/glad.h>

#include "platform/opengl/texture/GLTexture.h"

namespace Prehistoric
{
	class GLComputePipeline : public GLPipeline, public ComputePipeline
	{
	public:
		GLComputePipeline(Window* window, ResourceStorage* resourceStorage, ShaderHandle shader);
		virtual ~GLComputePipeline() {}

		virtual void BindPipeline(CommandBuffer* buffer) const override;
		virtual void RenderPipeline() const override;
		virtual void UnbindPipeline() const override;
	};
};

#endif