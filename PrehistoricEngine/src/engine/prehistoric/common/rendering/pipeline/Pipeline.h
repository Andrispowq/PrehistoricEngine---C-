#ifndef PIPELINE_H
#define PIPELINE_H

#include "prehistoric/common/buffer/VertexBuffer.h"
#include "prehistoric/common/rendering/shaders/Shader.h"

#include "prehistoric/core/util/math/Math.h"
#include "prehistoric/common/framework/Window.h"

namespace Prehistoric
{
	class AssetManager;

	class Pipeline
	{
	public:
		Pipeline(Window* window, AssetManager* manager, size_t shaderID);
		Pipeline() : samples(0), shaderID(-1) {}

		virtual ~Pipeline();

		virtual void BindPipeline(CommandBuffer* buffer) const = 0;
		virtual void RenderPipeline() const = 0;
		virtual void UnbindPipeline() const = 0;

		virtual void RecreatePipeline() {};

		Vector2f getViewportStart() const { return viewportStart; }
		Vector2f getViewportSize() const { return viewportSize; }
		Vector2u getScissorStart() const { return scissorStart; }
		Vector2u getScissorSize() const { return scissorSize; }

		int getSamples() const { return samples; }

		size_t getShaderID() const { return shaderID; }
		Shader* getShader() const;

		void setViewportStart(const Vector2f& viewportStart) { this->viewportStart = viewportStart; }
		void setViewportSize(const Vector2f& viewportSize) { this->viewportSize = viewportSize; }
		void setScissorStart(const Vector2u& scissorStart) { this->scissorStart = scissorStart; }
		void setScissorSize(const Vector2u& scissorSize) { this->scissorSize = scissorSize; }
		void setSamples(int samples) { this->samples = samples; }

		void setShaderID(size_t shaderID);

	public:
		Window* window;
		AssetManager* assetManager;

		size_t shaderID;

		Vector2f viewportStart;
		Vector2f viewportSize;
		Vector2u scissorStart;
		Vector2u scissorSize;

		int samples;
	};
};

#endif