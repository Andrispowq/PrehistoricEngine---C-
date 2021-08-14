#ifndef GL_BLOOM_DECOMPOSE_SHADER_H
#define GL_BLOOM_DECOMPOSE_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLBloomDecomposeShader : public GLShader
	{
	public:
		GLBloomDecomposeShader();
		virtual ~GLBloomDecomposeShader() {}

		void UpdateUniforms(Texture* scene, float threshold) const;
	};
};

#endif