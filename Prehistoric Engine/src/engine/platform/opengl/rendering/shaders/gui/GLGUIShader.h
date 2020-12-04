#ifndef GL_GUI_SHADER_H
#define GL_GUI_SHADER_H

#include <glew.h>

#include "engine/platform/opengl/rendering/shaders/GLShader.h"

class GLGUIShader : public GLShader
{
public:
	GLGUIShader();
	virtual ~GLGUIShader() {}

	virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
};

#endif