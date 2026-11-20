#ifndef GL_HKT_SHADER_H
#define GL_HKT_SHADER_H

#include "Includes.hpp"

#include <glad/glad.h>

#include "platform/opengl/rendering/shaders/GLShader.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLHktShader : public GLShader
	{
	public:
		GLHktShader();
		virtual ~GLHktShader() {}

		void UpdateUniforms(int N, int L, float t) const;
	};
};

#endif