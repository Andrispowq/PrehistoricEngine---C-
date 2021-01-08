#ifndef MESH_VERTEX_BUFFER_H
#define MESH_VERTEX_BUFFER_H

#include "VertexBuffer.h"

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/core/model/Mesh.h"

namespace Prehistoric
{
	class MeshVertexBuffer : public VertexBuffer
	{
	public:
		MeshVertexBuffer(Window* window, const Mesh& mesh) : VertexBuffer(window), mesh(mesh) {}

		virtual ~MeshVertexBuffer() override = 0;

		virtual void Bind(CommandBuffer* commandBuffer) const override = 0;
		virtual void Draw(CommandBuffer* commandBuffer) const = 0;
		virtual void Unbind() const override = 0;
	private:
		Mesh mesh;
	};
};

#endif