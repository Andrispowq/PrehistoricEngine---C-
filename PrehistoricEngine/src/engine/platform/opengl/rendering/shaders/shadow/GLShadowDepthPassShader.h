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

		virtual void UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;

	private:
		int m_transform;
	};
};

#endif