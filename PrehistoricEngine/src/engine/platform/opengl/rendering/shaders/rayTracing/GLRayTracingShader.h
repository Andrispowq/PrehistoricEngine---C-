#ifndef GL_RAY_TRACING_SHADER_H
#define GL_RAY_TRACING_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class Renderer;

	class GLRayTracingShader : public GLShader
	{
	public:
		GLRayTracingShader();
		virtual ~GLRayTracingShader() {}

		void UpdateUniforms(Renderer* renderer, uint32_t current_spp) const;
	};
};

#endif