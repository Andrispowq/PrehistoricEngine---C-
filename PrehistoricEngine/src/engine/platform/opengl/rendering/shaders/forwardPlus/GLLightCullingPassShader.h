#ifndef GL_LIGHT_CULLING_PASS_SHADER_H
#define GL_LIGHT_CULLING_PASS_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLLightCullingPassShader : public GLShader
	{
	public:
		GLLightCullingPassShader();
		virtual ~GLLightCullingPassShader() {}

		void UpdateUniforms(Camera* camera, const std::vector<Light*>& lights, Texture* depthMap);
	};
};

#endif