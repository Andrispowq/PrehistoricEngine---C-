#ifndef GL_SHADOW_DEPTH_PASS_SHADER_H
#define GL_SHADOW_DEPTH_PASS_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/core/node/GameObject.h"
#include "prehistoric/common/buffer/UniformBufferObject.h"

namespace Prehistoric
{
	class GLShadowDepthPassShader : public GLShader
	{
	public:
		GLShadowDepthPassShader();
		virtual ~GLShadowDepthPassShader() {}

		void UpdateGlobalUniforms(UniformBufferObject* matrices);

	private:
		int m_transform;
	};
};

#endif