#ifndef GL_IRRADIANCE_SHADER_H
#define GL_IRRADIANCE_SHADER_H

#include "engine/platform/opengl/rendering/shaders/GLShader.h"

#include <glew.h>

#include "engine/prehistoric/common/texture/Texture.h"

namespace Prehistoric
{
	class GLIrradianceShader : public GLShader
	{
	public:
		GLIrradianceShader();
		virtual ~GLIrradianceShader() {}

		void UpdateUniforms(const Matrix4f& projection, const Matrix4f& view, Texture* texture) const;
	};
};

#endif