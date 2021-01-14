#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/common/texture/Texture.h"

#include "platform/opengl/buffer/GLMeshVertexBuffer.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

#include "platform/opengl/rendering/shaders/GLShader.h"
#include "platform/vulkan/rendering/shaders/VKShader.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"
#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/core/util/ModelFabricator.h"

namespace Prehistoric
{
	enum class GUIType
	{
		Element, Button, Slider
	};

	class GUIElement : public GameObject
	{
	public:
		GUIElement(Window* window, AssembledAssetManager* manager, Vector3f colour = -1, void* data = nullptr, size_t dataSize = 0, bool visible = true);
		virtual ~GUIElement() {}

		virtual void PreUpdate(CoreEngine* engine) override;
		virtual void PreRender(Renderer* renderer) override;

		bool inside(Vector2f cursor, Vector2f _start = -1, Vector2f _end = -1);

		bool isVisible() const { return visible; }
		bool hasChanged() const { return statusChanged; }
		Vector3f getColour() { return colour; }

		void* getData() { return data; }
		size_t getDataSize() const { return dataSize; }
		GUIType getType() const { return type; }

		void setVisible(bool visible) { this->visible = visible; }
		void setColour(Vector3f colour) { this->colour = colour; }

		void setData(void* data, size_t dataSize = 0) { this->data = data; this->dataSize = dataSize; }

		GUIElement(const GUIElement&) = default;
	protected:
		Window* window;
		AssembledAssetManager* manager;
		GUIType type;

		bool visible;
		bool statusChanged;
		Vector3f colour;

		void* data;
		size_t dataSize;

		static VertexBufferHandle vbo;
		static PipelineHandle pipeline;
	};
};

#endif