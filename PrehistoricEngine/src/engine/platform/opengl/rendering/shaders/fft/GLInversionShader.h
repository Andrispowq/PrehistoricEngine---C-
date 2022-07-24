#ifndef GL_INVERSION_SHADER_H
#define GL_INVERSION_SHADER_H

#include "Includes.hpp"

#include <glad/glad.h>

#include "platform/opengl/rendering/shaders/GLShader.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLInversionShader : public GLShader
	{
	public:
		GLInversionShader();
		virtual ~GLInversionShader() {}

		void UpdateUniforms(int N, int pingpong) const;
	};
};

#endif