#include "Includes.hpp"
#include "GUIElement.h"

#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"
//#include "platform/vulkan/rendering/shaders/basic/VKGUIShader.h"

namespace Prehistoric
{
	VertexBufferHandle GUIElement::vbo = { nullptr, -1 };
	PipelineHandle GUIElement::pipeline = { nullptr, -1 };

	GUIElement::GUIElement(Window* window, AssembledAssetManager* manager, Vector3f colour, void* data, size_t dataSize, bool visible)
		: window(window), manager(manager), colour(colour), data(data), dataSize(dataSize), visible(visible), type(GUIType::Element)
	{
		if (vbo.handle == -1)
		{
			ShaderHandle shader;

			vbo = manager->getAssetManager()->storeVertexBuffer(ModelFabricator::CreateQuad(window));
			vbo->setFrontFace(FrontFace::DOUBLE_SIDED);

			if (FrameworkConfig::api == OpenGL)
			{
				shader = manager->getAssetManager()->loadShader(ShaderName::Gui).value();
				pipeline = manager->storePipeline(new GLGraphicsPipeline(window, manager->getAssetManager(), shader, vbo));
			}
			else if (FrameworkConfig::api == Vulkan)
			{
				//shader = manager->getAssetManager()->loadShader("gui").value();
				//pipeline = manager->storePipeline(new VKGraphicsPipeline(window, manager->getAssetManager(), shader, vbo));
			}
		}

		RendererComponent* renderer = new RendererComponent(window, manager, pipeline, manager->storeMaterial(nullptr));
		renderer->setPriority(RenderPriority::_2D);

		AddComponent(RENDERER_COMPONENT, renderer);
	}

	void GUIElement::PreUpdate(CoreEngine* engine)
	{
	}

	void GUIElement::PreRender(Renderer* renderer)
	{
		if (!visible)
			return;

		GameObject::PreRender(renderer);
	}

	bool GUIElement::inside(Vector2f cursor, Vector2f _start, Vector2f _end)
	{
		Vector2f pos = worldTransform.getPosition().xy();
		Vector2f scale = worldTransform.getScaling().xy();

		Vector2f start = pos - scale;
		Vector2f end = pos + scale;

		if (_start != -1 && _end != -1)
		{
			start = _start;
			end = _end;
		}

		cursor.y = window->getHeight() - cursor.y;
		cursor /= { (float)window->getWidth(), (float)window->getHeight() };
		cursor *= 2;
		cursor -= 1;

		if ((start <= cursor) && (end >= cursor))
		{
			return true;
		}

		return false;
	}
};