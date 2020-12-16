#ifndef GL_SPLAT_MAP_SHADER_H
#define GL_SPLAT_MAP_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLSplatMapShader : public GLShader
	{
	public:
		GLSplatMapShader();
		virtual ~GLSplatMapShader() {}

		void UpdateUniforms(Texture* texture, uint32_t N) const;
	};
};

#endif