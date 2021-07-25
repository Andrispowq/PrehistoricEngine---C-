#ifndef GL_BLOOM_DESCALE_SHADER_H
#define GL_BLOOM_DESCALE_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLBloomDescaleShader : public GLShader
	{
	public:
		GLBloomDescaleShader();
		virtual ~GLBloomDescaleShader() {}

		void UpdateUniforms(Texture* scene) const;
	};
};

#endif