#ifndef GL_HDR_SHADER_H
#define GL_HDR_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLHDRShader : public GLShader
	{
	public:
		GLHDRShader();
		virtual ~GLHDRShader() {}

		void UpdateUniforms(Texture* scene, Vector2f resolution) const;
	};
};

#endif