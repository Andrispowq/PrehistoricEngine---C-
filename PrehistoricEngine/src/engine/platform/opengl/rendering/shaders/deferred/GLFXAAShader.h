#ifndef GL_FXAA_SHADER_H
#define GL_FXAA_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLFXAAShader : public GLShader
	{
	public:
		GLFXAAShader();
		virtual ~GLFXAAShader() {}

		void UpdateUniforms(Renderer* renderer, Camera* camera, const std::vector<Light*>& lights) const;
	};
};

#endif