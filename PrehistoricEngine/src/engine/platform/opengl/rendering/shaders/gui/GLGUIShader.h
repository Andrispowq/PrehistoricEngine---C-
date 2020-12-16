#ifndef GL_GUI_SHADER_H
#define GL_GUI_SHADER_H

#include <glad/glad.h>

#include "platform/opengl/rendering/shaders/GLShader.h"

namespace Prehistoric
{
	class GLGUIShader : public GLShader
	{
	public:
		GLGUIShader();
		virtual ~GLGUIShader() {}

		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
	};
};

#endif