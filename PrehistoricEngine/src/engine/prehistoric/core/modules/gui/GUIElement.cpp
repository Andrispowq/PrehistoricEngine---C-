#include "Includes.hpp"
#include "GUIElement.h"

#include "prehistoric/core/util/ModelFabricator.h"

#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"
#include "platform/vulkan/rendering/shaders/basic/VKBasicShader.h"

#include "prehistoric/core/CoreEngine.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	VertexBufferHandle GUIElement::vbo = { nullptr, -1 };
	PipelineHandle GUIElement::pipeline = { nullptr, -1 };

	GUIElement::GUIElement(Window* window, AssembledAssetManager* manager, Texture* texture, void* data, size_t dataSize, bool visible)
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

			vbo = manager->getAssetManager()->loadVertexBuffer(std::nullopt, "res/models/quad.obj").value();
			vbo->setFrontFace(FrontFace::CLOCKWISE);

			if (FrameworkConfig::api == OpenGL)
			{
				shader = manager->getAssetManager()->loadShader("gui").value();
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