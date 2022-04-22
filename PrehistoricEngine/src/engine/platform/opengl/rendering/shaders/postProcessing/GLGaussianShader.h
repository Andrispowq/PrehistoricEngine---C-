#ifndef GL_GAUSSIAN_SHADER_H
#define GL_GAUSSIAN_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

namespace Prehistoric
{
	class GLGaussianShader : public GLShader
	{
	public:
		GLGaussianShader();
		virtual ~GLGaussianShader() {}

		void UpdateUniforms(Texture* scene, bool horizontal, Vector2f targetSize, Vector2f sceneSize) const;
	};
};

#endif