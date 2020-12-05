#ifndef MESH_VERTEX_BUFFER_H
#define MESH_VERTEX_BUFFER_H

#include "VertexBuffer.h"

#include "engine/prehistoric/core/model/Mesh.h"

namespace Prehistoric
{
	class MeshVertexBuffer : public VertexBuffer
	{
	public:
		MeshVertexBuffer(const Mesh& mesh) : mesh(mesh) {}

		virtual ~MeshVertexBuffer() override = 0;

		virtual void Bind(CommandBuffer* commandBuffer) const override = 0;
		virtual void Draw(CommandBuffer* commandBuffer) const = 0;
		virtual void Unbind() const override = 0;
	private:
		Mesh mesh;
	};
};

#endif