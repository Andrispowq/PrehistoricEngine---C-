#ifndef GL_ALPHA_COVERAGE_SHADER_H
#define GL_ALPHA_COVERAGE_SHADER_H

#include "platform/opengl/rendering/shaders/GLShader.h"

#include <glad/glad.h>

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	class GLAlphaCoverageShader : public GLShader
	{
	public:
		GLAlphaCoverageShader();
		virtual ~GLAlphaCoverageShader() {}

		void UpdateUniforms(Renderer* renderer, Camera* camera, const std::vector<Light*>& lights) const;
	};
};

#endif