#include "Includes.hpp"
#include "PatchVertexBuffer.h"

namespace Prehistoric
{
	PatchVertexBuffer::PatchVertexBuffer(Window* window, const std::vector<Vector2f>& vertices)
		: VertexBuffer(window), vertices(vertices)
	{
		vertexCount = vertices.size();
	}

	PatchVertexBuffer::~PatchVertexBuffer()
	{
	}
};