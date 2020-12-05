#ifndef GL_ENVIROMENT_MAP_SHADER_H
#define GL_ENVIROMENT_MAP_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glew.h>

#include "prehistoric/common/texture/Texture.h"

namespace Prehistoric
{
	class GLEnvironmentMapShader : public GLShader
	{
	public:
		GLEnvironmentMapShader();
		virtual ~GLEnvironmentMapShader() {}

		void UpdateUniforms(const Matrix4f& projection, const Matrix4f& view, Texture* texture) const;
	};
};

#endif