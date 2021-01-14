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

		void UpdateCustomUniforms(Texture* texture, Vector3f colour) const;
		virtual void UpdateObjectUniforms(GameObject* object, uint32_t instance_index = 0) const override;
	};
};

#endif