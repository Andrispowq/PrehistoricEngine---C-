#ifndef GL_COMPUTE_PIPELINE
#define GL_COMPUTE_PIPELINE

#include "prehistoric/common/rendering/pipeline/ComputePipeline.h"
#include "GLPipeline.h"

#include <glew.h>

#include "platform/opengl/texture/GLTexture.h"

namespace Prehistoric
{
	class GLComputePipeline : public GLPipeline, public ComputePipeline
	{
	public:
		GLComputePipeline(Window* window, AssetManager* manager, size_t shaderID);
		virtual ~GLComputePipeline() {}

		virtual void BindPipeline(CommandBuffer* buffer) const override;
		virtual void RenderPipeline() const override;
		virtual void UnbindPipeline() const override;
	};
};

#endif