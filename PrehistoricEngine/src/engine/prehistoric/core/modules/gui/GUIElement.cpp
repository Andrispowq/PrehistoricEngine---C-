#include "Includes.hpp"
#include "GUIElement.h"

#include "prehistoric/core/util/ModelFabricator.h"

#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"
#include "platform/vulkan/rendering/shaders/basic/VKBasicShader.h"

#include "prehistoric/core/Engine.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	size_t GUIElement::vboID = -1;
	size_t GUIElement::pipelineID = -1;

	GUIElement::GUIElement(Window* window, AssembledAssetManager* manager, Texture* texture, void* data, size_t dataSize, bool visible)
		: type(GUIType::Element)
	{
		this->texture = texture;
		this->data = data;
		this->dataSize = dataSize;
		this->visible = visible;

		this->window = window;

		if (vboID == -1)
		{
			vboID = manager->getAssetManager()->addResource<VertexBuffer>(ModelFabricator::CreateQuad(window));
			manager->getAssetManager()->getResourceByID<VertexBuffer>(vboID)->setFrontFace(FrontFace::CLOCKWISE);
		}

		if (pipelineID == -1)
		{
			size_t shaderID;

			if (FrameworkConfig::api == OpenGL)
			{
				shaderID = manager->getAssetManager()->getResource<Shader>("gui");
				pipelineID = manager->loadResource<Pipeline>(new GLGraphicsPipeline(window, manager->getAssetManager(), shaderID, vboID));
			}
			else if (FrameworkConfig::api == Vulkan)
			{
				shaderID = manager->getAssetManager()->getResource<Shader>("basic");
				pipelineID = manager->loadResource<Pipeline>(new VKGraphicsPipeline(window, manager->getAssetManager(), shaderID, vboID));
			}
		}

		RendererComponent* renderer = new RendererComponent(pipelineID, manager->loadResource<Material>(nullptr), window, manager);
		renderer->setPriority(RenderPriority::_2D);

		AddComponent(RENDERER_COMPONENT, renderer);
	}

	void GUIElement::PreUpdate(Engine* engine)
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

	GUIElement::~GUIElement()
	{
		//this data is most likely getting a pointer to a stack variable, so it is not ideal to delete it
		//delete data;
	}
};