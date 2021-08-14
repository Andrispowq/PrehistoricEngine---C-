#ifndef GL_BLOOM_COMBINE_SHADER_H
#define GL_BLOOM_COMBINE_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLBloomCombineShader : public GLShader
	{
	public:
		GLBloomCombineShader();
		virtual ~GLBloomCombineShader() {}

		void UpdateUniforms(Texture* tex0, Texture* tex1) const;
	};
};

#endif