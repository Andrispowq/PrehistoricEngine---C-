#ifndef VertexBuffer_H
#define VertexBuffer_H

#include "prehistoric/core/model/Mesh.h"

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/common/rendering/command/CommandBuffer.h"

namespace Prehistoric
{
	enum class FrontFace
	{
		CLOCKWISE, COUNTER_CLOCKWISE, DOUBLE_SIDED
	};

	class VertexBuffer
	{
	public:
		VertexBuffer(Window* window) : window(window), size(0), indexed(false), frontFace(FrontFace::CLOCKWISE) {}
		virtual ~VertexBuffer() = 0;

		virtual void Bind(CommandBuffer* commandBuffer) const = 0;
		virtual void Draw(CommandBuffer* commandBuffer) const = 0;
		virtual void Unbind() const = 0;

		FrontFace getFrontFace() const { return frontFace; }
		void setFrontFace(FrontFace frontFace) { this->frontFace = frontFace; }

		//We should avoid copying this class, because it manages some GPU resources
		VertexBuffer(VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&&) = delete;
		VertexBuffer& operator=(VertexBuffer) = delete;
	protected:
		Window* window;

		uint32_t size;
		bool indexed;

		FrontFace frontFace;
	};
};

#endif