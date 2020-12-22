#ifndef GL_DEFERRED_SHADER_H
#define GL_DEFERRED_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLDeferredShader : public GLShader
	{
	public:
		GLDeferredShader();
		virtual ~GLDeferredShader() {}

		void UpdateUniforms(Renderer* renderer, Camera* camera, const std::vector<Light*>& lights) const;
	};
};

#endif