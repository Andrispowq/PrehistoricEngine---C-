#ifndef GL_SPLAT_MAP_SHADER_H
#define GL_SPLAT_MAP_SHADER_H

#include "engine/platform/opengl/rendering/shaders/GLShader.h"

#include <glew.h>

class GLSplatMapShader : public GLShader
{
public:
	GLSplatMapShader();
	virtual ~GLSplatMapShader() {}

	void UpdateUniforms(Texture* texture, uint32_t N) const;
};

#endif