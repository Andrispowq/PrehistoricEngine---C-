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

	enum class VertexBufferType
	{
		MESH, PATCH
	};

	class VertexBuffer
	{
	public:
		VertexBuffer(Window* window) : window(window), indexed(false), frontFace(FrontFace::CLOCKWISE), size(0), submeshCount(0) {}
		virtual ~VertexBuffer() = 0;

		virtual void Bind(CommandBuffer* commandBuffer) const = 0;
		virtual void Draw(CommandBuffer* commandBuffer, uint32_t submesh) const = 0;
		virtual void Unbind() const = 0;

		FrontFace getFrontFace() const { return frontFace; }
		void setFrontFace(FrontFace frontFace) { this->frontFace = frontFace; }

		uint32_t getSubmeshCount() const { return submeshCount; }

		VertexBufferType getType() const { return type; }

		//We should avoid copying this class, because it manages some GPU resources
		VertexBuffer(VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&&) = delete;
		VertexBuffer& operator=(VertexBuffer) = delete;
	protected:
		Window* window;

		uint32_t size;
		uint32_t submeshCount;
		bool indexed;

		FrontFace frontFace;
		VertexBufferType type;
	};
};

#endif