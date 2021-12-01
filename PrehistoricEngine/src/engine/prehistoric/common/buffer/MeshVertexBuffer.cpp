#include "Includes.hpp"
#include "MeshVertexBuffer.h"

namespace Prehistoric
{
	MeshVertexBuffer::MeshVertexBuffer(Window* window, const Model& model)
		: VertexBuffer(window), model(model)
	{
		type = VertexBufferType::MESH;
		submeshCount = (uint32_t)model.getMeshes().size();
	}

	MeshVertexBuffer::~MeshVertexBuffer()
	{
	}
};