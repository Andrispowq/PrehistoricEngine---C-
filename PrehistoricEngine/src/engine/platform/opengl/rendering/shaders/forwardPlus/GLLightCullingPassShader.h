#ifndef GL_LIGHT_CULLING_PASS_SHADER_H
#define GL_LIGHT_CULLING_PASS_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLLightCullingPassShader : public GLShader
	{
	public:
		GLLightCullingPassShader();
		virtual ~GLLightCullingPassShader() {}

		void UpdateUniforms(Renderer* renderer, Texture* depthMap);
	};
};

#endif