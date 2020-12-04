#ifndef GL_BRDF_INTEGRATE_SHADER_H
#define GL_BRDF_INTEGRATE_SHADER_H

#include "engine/platform/opengl/rendering/shaders/GLShader.h"

#include <glew.h>

#include "engine/prehistoric/common/texture/Texture.h"

class GLBRDFIntegrateShader : public GLShader
{
public:
	GLBRDFIntegrateShader();
	virtual ~GLBRDFIntegrateShader() {}
};

#endif