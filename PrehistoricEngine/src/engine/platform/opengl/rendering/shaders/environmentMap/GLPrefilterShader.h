#ifndef GL_PREFILTER_SHADER_H
#define GL_PREFILTER_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/common/texture/Texture.h"

namespace Prehistoric
{
	class GLPrefilterShader : public GLShader
	{
	public:
		GLPrefilterShader();
		virtual ~GLPrefilterShader() {}

		void UpdateUniforms(const Matrix4f& projection, const Matrix4f& view, Texture* texture, float roughness) const;
	};
};

#endif