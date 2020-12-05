#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include "engine/prehistoric/core/node/GameObject.h"

#include "engine/prehistoric/common/texture/Texture.h"

#include "engine/platform/opengl/buffer/GLMeshVertexBuffer.h"
#include "engine/platform/vulkan/buffer/VKMeshVertexBuffer.h"

#include "engine/platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "engine/platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

#include "engine/platform/opengl/rendering/shaders/GLShader.h"
#include "engine/platform/vulkan/rendering/shaders/VKShader.h"

#include "engine/prehistoric/core/node/component/renderer/RendererComponent.h"

namespace Prehistoric
{
	enum class GUIType
	{
		Element, Button, Slider
	};

	class GUIElement : public GameObject
	{
	public:
		GUIElement(Window* window, AssembledAssetManager* manager, Texture* texture = nullptr, void* data = nullptr, size_t dataSize = 0, bool visible = true);
		virtual ~GUIElement();

		virtual void PreUpdate(Engine* engine) override;
		virtual void PreRender(Renderer* renderer) override;

		bool inside(Vector2f cursor);

		bool isVisible() const { return visible; }
		bool hasChanged() const { return statusChanged; }

		void* getData() { return data; }
		size_t getDataSize() const { return dataSize; }

		Texture* getTexture() { return texture; }

		GUIType getType() const { return type; }

		void setVisible(bool visible) { this->visible = visible; }

		void setData(void* data, size_t dataSize = 0) { this->data = data; this->dataSize = dataSize; }
		void setTexture(Texture* texture) { this->texture = texture; }

		GUIElement(const GUIElement&) = default;
	protected:
		GUIType type;

		bool visible;
		bool statusChanged;

		void* data;
		size_t dataSize;

		static size_t vboID;
		static size_t pipelineID;

		Texture* texture;

		Window* window;
	};
};

#endif