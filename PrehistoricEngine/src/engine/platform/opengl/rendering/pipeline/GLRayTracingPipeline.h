#ifndef GL_RAY_TRACING_PIPELINE
#define GL_RAY_TRACING_PIPELINE

#include "prehistoric/common/rendering/pipeline/RayTracingPipeline.h"
#include "GLPipeline.h"

#include <glad/glad.h>

#include "platform/opengl/texture/GLTexture.h"

namespace Prehistoric
{
	class GLRayTracingPipeline : public GLPipeline, public RayTracingPipeline
	{
	public:
		GLRayTracingPipeline(Window* window, AssetManager* manager, ShaderHandle shader);
		virtual ~GLRayTracingPipeline() {}

		virtual void BindPipeline(CommandBuffer* buffer) const override;
		virtual void RenderPipeline() const override;
		virtual void UnbindPipeline() const override;
	};
};

#endif