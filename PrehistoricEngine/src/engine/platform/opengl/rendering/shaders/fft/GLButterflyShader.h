#ifndef GL_BUTTERFLY_SHADER_H
#define GL_BUTTERFLY_SHADER_H

#include "Includes.hpp"

#include <glad/glad.h>

#include "platform/opengl/rendering/shaders/GLShader.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLButterflyShader : public GLShader
	{
	public:
		GLButterflyShader();
		virtual ~GLButterflyShader() {}

		void UpdateUniforms(int pingpong, int direciton, int stage) const;
	};
};

#endif