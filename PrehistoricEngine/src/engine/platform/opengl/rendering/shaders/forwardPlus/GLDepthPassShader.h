#ifndef GL_DEPTH_PASS_SHADER_H
#define GL_DEPTH_PASS_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric
{
	class GLDepthPassShader : public GLShader
	{
	public:
		GLDepthPassShader();
		virtual ~GLDepthPassShader() {}

		virtual void UpdateGlobalUniforms(Camera* camera, const std::vector<Light*>& lights) const override;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;

	private:
		int m_transform;
	};
};

#endif