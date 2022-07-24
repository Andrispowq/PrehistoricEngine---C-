#ifndef GL_TWIDDLE_FACTORS_SHADER_H
#define GL_TWIDDLE_FACTORS_SHADER_H

#include "Includes.hpp"

#include <glad/glad.h>

#include "platform/opengl/rendering/shaders/GLShader.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLTwiddleFactorsShader : public GLShader
	{
	public:
		GLTwiddleFactorsShader();
		virtual ~GLTwiddleFactorsShader() {}

		void UpdateUniforms(int N) const;
	};
};

#endif