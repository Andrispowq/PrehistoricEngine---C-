#ifndef PATCH_VERTEX_BUFFER_H
#define PATCH_VERTEX_BUFFER_H

#include "Includes.hpp"

#include "VertexBuffer.h"

namespace Prehistoric
{
	class PatchVertexBuffer : public VertexBuffer
	{
	public:
		PatchVertexBuffer(Window* window, const std::vector<Vector2f>& vertices);
		virtual ~PatchVertexBuffer() override = 0;

		virtual void Bind(CommandBuffer* commandBuffer) const override = 0;
		virtual void Draw(CommandBuffer* commandBuffer, uint32_t submesh) const = 0;
		virtual void Unbind() const override = 0;
	protected:
		std::vector<Vector2f> vertices;

		size_t vertexCount;
	};
};

#endif