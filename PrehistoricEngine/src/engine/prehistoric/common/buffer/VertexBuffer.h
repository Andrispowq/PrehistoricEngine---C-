#ifndef VertexBuffer_H
#define VertexBuffer_H

#include "prehistoric/core/model/Mesh.h"

#include "prehistoric/common/rendering/command/CommandBuffer.h"

#if !defined(HANDLE_OF)
#define HANDLE_OF(type) typedef struct type##_handle_t { type* pointer = nullptr; size_t handle = -1; type* operator->() { return pointer;}; type* const operator->() const { return pointer;} } type##Handle
#endif

namespace Prehistoric
{
	enum class FrontFace
	{
		CLOCKWISE, COUNTER_CLOCKWISE, DOUBLE_SIDED
	};

	class VertexBuffer
	{
	public:
		VertexBuffer() : size(0), indexed(false), frontFace(FrontFace::COUNTER_CLOCKWISE) {}
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
		uint32_t size;
		bool indexed;

		FrontFace frontFace;
	};

	HANDLE_OF(VertexBuffer);
};

#endif