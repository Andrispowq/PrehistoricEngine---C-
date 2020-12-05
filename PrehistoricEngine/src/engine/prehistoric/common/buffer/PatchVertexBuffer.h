#ifndef PATCH_VERTEX_BUFFER_H
#define PATCH_VERTEX_BUFFER_H

#include "Includes.hpp"

#include "VertexBuffer.h"

namespace Prehistoric
{
	class PatchVertexBuffer : public VertexBuffer
	{
	public:
		PatchVertexBuffer(const std::vector<Vector2f>& vertices) : vertices(vertices) {}
		virtual ~PatchVertexBuffer() override = 0;

		virtual void Bind(CommandBuffer* commandBuffer) const override = 0;
		virtual void Draw(CommandBuffer* commandBuffer) const = 0;
		virtual void Unbind() const override = 0;
	private:
		std::vector<Vector2f> vertices;
	};
};

#endif