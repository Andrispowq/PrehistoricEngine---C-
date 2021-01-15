#include "Includes.hpp"
#include "MeshVertexBuffer.h"

namespace Prehistoric
{
	MeshVertexBuffer::MeshVertexBuffer(Window* window, const Mesh& mesh)
		: VertexBuffer(window), mesh(mesh)
	{
		vertices = mesh.getVertices().size();
		indices = mesh.getIndices().size();
	}

	MeshVertexBuffer::~MeshVertexBuffer()
	{

	}
};