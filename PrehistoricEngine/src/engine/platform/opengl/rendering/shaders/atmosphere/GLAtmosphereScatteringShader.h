#ifndef GL_ATMOSPHERE_SCATTERING_SHADER_H
#define GL_ATMOSPHERE_SCATTERING_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLAtmosphereScatteringShader : public GLShader
	{
	public:
		GLAtmosphereScatteringShader();
		virtual ~GLAtmosphereScatteringShader() {}

		void UpdateUniforms(const Vector3f& sun, const Matrix4f& transform, const Matrix4f& view,
			const Matrix4f& projection, bool capture) const;
	};
};

#endif