#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLGUIShader.h"

#include "engine/prehistoric/core/modules/gui/GUIElement.h"
#include "engine/prehistoric/core/modules/gui/slider/GUISlider.h"

GLGUIShader::GLGUIShader()
	: GLShader()
{
	AddShader(ResourceLoader::LoadShaderGL("opengl/gui/gui_VS.glsl"), VERTEX_SHADER);
	AddShader(ResourceLoader::LoadShaderGL("opengl/gui/gui_FS.glsl"), FRAGMENT_SHADER);
	CompileShader();

	AddUniform("m_transform");

	AddUniform("image");
	AddUniform("progress");
}

void GLGUIShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
{
	SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix());

	GUIElement* gui = reinterpret_cast<GUIElement*>(object);

	gui->getTexture()->Bind();
	SetUniformi("image", 0);

	if (gui->getType() == GUIType::Slider)
	{
		SetUniformf("progress", reinterpret_cast<GUISlider*>(object)->getProgress());
	}
	else
	{
		SetUniformf("progress", 1.0f);
	}
}