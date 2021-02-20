#ifndef PIPELINE_H
#define PIPELINE_H

#include "prehistoric/common/buffer/VertexBuffer.h"
#include "prehistoric/common/rendering/shaders/Shader.h"

#include "prehistoric/core/util/math/Math.h"
#include "prehistoric/common/framework/Window.h"

#include "prehistoric/core/resources/AssetManager.h"

namespace Prehistoric
{
	enum class APIHashFlags
	{
		GL = 0x0,
		VK = 0x1,
		DX11 = 0x2,
		DX12 = 0x3
	};

	enum class PipelineTypeHashFlags
	{
		Graphics = 0x0,
		Compute = 0x1,
		RayTracing = 0x2
	};

	class Pipeline
	{
	public:
		Pipeline(Window* window, AssetManager* manager, ShaderHandle shader);
		Pipeline() : samples(0) {}

		virtual ~Pipeline();

		virtual void BindPipeline(CommandBuffer* buffer) const = 0;
		virtual void RenderPipeline() const = 0;
		virtual void UnbindPipeline() const = 0;

		virtual void RecreatePipeline() {};

		virtual uint64_t GetHash();

		static uint64_t GetHash(APIHashFlags apiFlags, PipelineTypeHashFlags typeFlags, size_t shaderHandle, size_t otherHandle);

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

	protected:
		void SetHashInternal(APIHashFlags apiFlags, PipelineTypeHashFlags typeFlags, size_t shaderHandle, size_t otherHandle);

	public:
		Window* window;
		AssetManager* manager;

		ShaderHandle shader;

		Vector2f viewportStart;
		Vector2f viewportSize;
		Vector2u scissorStart;
		Vector2u scissorSize;

		int samples;

		uint64_t hash = 0;

	protected:
		mutable CommandBuffer* buffer; //The current commandbuffer
	};
};

#endif