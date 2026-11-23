#ifndef GL_ATMOSPHERE_SHADER_H
#define GL_ATMOSPHERE_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLAtmosphereShader : public GLShader
	{
	public:
		GLAtmosphereShader();
		virtual ~GLAtmosphereShader() {}

		void UpdateUniforms(const Matrix4f& transform, const Matrix4f& view, const Matrix4f& projection) const;
	};
};

#endif