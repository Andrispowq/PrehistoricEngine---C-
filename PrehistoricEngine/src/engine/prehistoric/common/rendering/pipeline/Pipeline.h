#ifndef PIPELINE_H
#define PIPELINE_H

#include "prehistoric/common/buffer/VertexBuffer.h"
#include "prehistoric/common/rendering/shaders/Shader.h"

#include "prehistoric/core/util/math/Math.h"
#include "prehistoric/common/framework/Window.h"

#if !defined(HANDLE_OF)
#define HANDLE_OF(type) typedef struct type##_handle_t { type* pointer = nullptr; size_t handle = -1; type* operator->() { return pointer;}; type* const operator->() const { return pointer;} } type##Handle
#endif

namespace Prehistoric
{
	class ResourceStorage;

	class Pipeline
	{
	public:
		Pipeline(Window* window, ResourceStorage* resourceStorage, ShaderHandle shader);
		Pipeline() : samples(0) {}

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

		Shader* getShader() const { return shader.pointer; }
		ShaderHandle getShaderHandle() const { return shader; }

		void setViewportStart(const Vector2f& viewportStart) { this->viewportStart = viewportStart; }
		void setViewportSize(const Vector2f& viewportSize) { this->viewportSize = viewportSize; }
		void setScissorStart(const Vector2u& scissorStart) { this->scissorStart = scissorStart; }
		void setScissorSize(const Vector2u& scissorSize) { this->scissorSize = scissorSize; }
		void setSamples(int samples) { this->samples = samples; }

	public:
		Window* window;
		ResourceStorage* resourceStorage;

		ShaderHandle shader;

		Vector2f viewportStart;
		Vector2f viewportSize;
		Vector2u scissorStart;
		Vector2u scissorSize;

		int samples;

	protected:
		mutable CommandBuffer* buffer; //The current commandbuffer
	};

	HANDLE_OF(Pipeline);
};

#endif