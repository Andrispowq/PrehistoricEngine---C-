#include "Includes.hpp"
#include "GUIElement.h"

#include "prehistoric/core/util/ModelFabricator.h"

#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"
#include "platform/vulkan/rendering/shaders/basic/VKBasicShader.h"

#include "prehistoric/core/CoreEngine.h"

#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	VertexBufferHandle GUIElement::vbo = { nullptr, -1 };
	PipelineHandle GUIElement::pipeline = { nullptr, -1 };

	GUIElement::GUIElement(Window* window, ResourceStorage* resourceStorage, Texture* texture, void* data, size_t dataSize, bool visible)
		: type(GUIType::Element)
	{
		this->texture = texture;
		this->data = data;
		this->dataSize = dataSize;
		this->visible = visible;

		this->window = window;

		if (vbo.handle == -1)
		{
			ShaderHandle shader;

			vbo = resourceStorage->loadVertexBuffer(std::nullopt, "quad").value();
			vbo->setFrontFace(FrontFace::CLOCKWISE);

			if (FrameworkConfig::api == OpenGL)
			{
				shader = resourceStorage->loadShader("gui").value();
				pipeline = resourceStorage->storePipeline(new GLGraphicsPipeline(window, resourceStorage, shader, vbo));
			}
			else if (FrameworkConfig::api == Vulkan)
			{
				//shader = resourceStorage->loadShader("gui").value();
				//pipeline = resourceStorage->storePipeline(new VKGraphicsPipeline(window, resourceStorage, shader, vbo));
			}
		}

		RendererComponent* renderer = new RendererComponent(pipeline, resourceStorage->storeMaterial(nullptr), window, resourceStorage);
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

	bool GUIElement::inside(Vector2f cursor)
	{
		cursor.y = window->getHeight() - cursor.y;
		cursor /= { (float)window->getWidth(), (float)window->getHeight() };
		cursor *= 2;
		cursor -= 1;

		Vector2f pos = worldTransform.getPosition().xy();
		Vector2f scale = worldTransform.getScaling().xy();

		Vector2f start = pos - scale;
		Vector2f end = pos + scale;

		if (start <= cursor && end >= cursor)
		{
			return true;
		}

		return false;
	}
};