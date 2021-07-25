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

		void UpdateUniforms(Texture* scene, Texture* bloom[5], float bloom_factor) const;
	};
};

#endif