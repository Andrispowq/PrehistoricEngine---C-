#ifndef GL_NORMAL_MAP_SHADER_H
#define GL_NORMAL_MAP_SHADER_H

#include "engine/platform/opengl/rendering/shaders/GLShader.h"

#include <glew.h>

class GLNormalMapShader : public GLShader
{
public:
	GLNormalMapShader();
	virtual ~GLNormalMapShader() {}

	void UpdateUniforms(Texture* texture, uint32_t N, float strength) const;
};

#endif