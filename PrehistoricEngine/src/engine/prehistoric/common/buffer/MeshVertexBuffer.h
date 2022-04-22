#ifndef MESH_VERTEX_BUFFER_H
#define MESH_VERTEX_BUFFER_H

#include "VertexBuffer.h"

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/core/model/Model.h"

namespace Prehistoric
{
	class MeshVertexBuffer : public VertexBuffer
	{
	public:
		MeshVertexBuffer(Window* window, const Model& model);

		virtual ~MeshVertexBuffer() override = 0;

		virtual void Bind(CommandBuffer* commandBuffer) const override = 0;
		virtual void Draw(CommandBuffer* commandBuffer, uint32_t submesh) const = 0;
		virtual void Unbind() const override = 0;

		Model& getModel() { return model; }
	protected:
		Model model;

		std::vector<size_t> index_offsets;
		std::vector<size_t> sizes;
		std::vector<size_t> vertices;
	};
};

#endif