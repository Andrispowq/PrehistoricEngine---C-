#ifndef GL_H0K_SHADER_H
#define GL_H0K_SHADER_H

#include "Includes.hpp"

#include <glad/glad.h>

#include "platform/opengl/rendering/shaders/GLShader.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLH0kShader : public GLShader
	{
	public:
		GLH0kShader();
		virtual ~GLH0kShader() {}

		void UpdateUniforms(int N, int L, float amplitude, Vector2f direction,
			float alignment, float intensity, float capillarSupressFactor) const;
		void UpdateUniformsTextures(int t0, int t1, int t2, int t3) const;
	};
};

#endif