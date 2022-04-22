#include "Includes.hpp"
#include "GLGUIShader.h"

#include "prehistoric/core/modules/gui/GUIElement.h"
#include "prehistoric/core/modules/gui/slider/GUISlider.h"

namespace Prehistoric
{
	GLGUIShader::GLGUIShader()
		: GLShader()
	{
		AddShader(ResourceLoader::LoadShaderGL("opengl/gui/gui_VS.glsl"), VERTEX_SHADER);
		AddShader(ResourceLoader::LoadShaderGL("opengl/gui/gui_FS.glsl"), FRAGMENT_SHADER);
		CompileShader();

		AddUniform("m_transform");

		AddUniform("colour");
		AddUniform("image");
	}

	void GLGUIShader::UpdateCustomUniforms(Texture* texture, Vector3f colour) const
	{
		SetUniform("m_transform", Matrix4f::Identity());

		SetUniform("colour", colour);

		texture->Bind();
		SetUniformi("image", 0);
	}

	void GLGUIShader::UpdateObjectUniforms(GameObject* object, uint32_t instance_index) const
	{
		GUIElement* gui = reinterpret_cast<GUIElement*>(object);

		Texture* tex = gui->getTexture();
		if (tex)
		{
			tex->Bind(0);
			SetUniformi("image", 0);
		}

		if (gui->getType() == GUIType::Slider)
		{
			GUISlider* gui_slider = reinterpret_cast<GUISlider*>(gui);

			SetUniform("m_transform", gui_slider->getSliderTransform());
			SetUniform("colour", gui_slider->getSliderColour());
		}
		else
		{
			SetUniform("m_transform", object->getWorldTransform().getTransformationMatrix());
			SetUniform("colour", gui->getColour());
		}
	}
};